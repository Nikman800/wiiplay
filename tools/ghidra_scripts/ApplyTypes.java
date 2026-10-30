// Import build/ghidra/types.h (generated from the C++ headers via CodeWarrior debug info), type
// `this` (first parameter) of every function listed in build/ghidra/this_map.tsv, and type the
// global data listed in build/ghidra/globals.tsv (both written by tools/gen_this_map.py).
// Args: <types.h> <this_map.tsv> [globals.tsv]
// @category WiiPlay
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.app.util.cparser.C.CParser;
import ghidra.program.model.address.Address;
import ghidra.program.model.data.ArrayDataType;
import ghidra.program.model.data.BooleanDataType;
import ghidra.program.model.data.CharDataType;
import ghidra.program.model.data.DataType;
import ghidra.program.model.data.DataTypeManager;
import ghidra.program.model.data.DoubleDataType;
import ghidra.program.model.data.FloatDataType;
import ghidra.program.model.data.IntegerDataType;
import ghidra.program.model.data.PointerDataType;
import ghidra.program.model.data.ShortDataType;
import ghidra.program.model.data.SignedCharDataType;
import ghidra.program.model.data.UnsignedCharDataType;
import ghidra.program.model.data.UnsignedIntegerDataType;
import ghidra.program.model.data.UnsignedShortDataType;
import ghidra.program.model.data.VoidDataType;
import ghidra.program.model.data.WideChar16DataType;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Listing;
import ghidra.program.model.listing.Parameter;
import ghidra.program.model.pcode.HighFunction;
import ghidra.program.model.pcode.HighFunctionDBUtil;
import ghidra.program.model.symbol.SourceType;
import ghidra.program.model.symbol.Symbol;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ApplyTypes extends GhidraScript {
	private final Map<String, DataType> structs = new HashMap<>();
	private int ambiguous = 0;

	@Override
	protected void run() throws Exception {
		String[] args = getScriptArgs();
		DataTypeManager dtm = currentProgram.getDataTypeManager();
		// parse the forward declarations, then one struct definition per block (a whole-file parse
		// gives no position on failure, and one bad definition would lose every type)
		String[] blocks = Files.readString(Path.of(args[0])).split("\n(?=(?:struct|union) \\w+ \\{)");
		// one parser for all blocks: a fresh parser per block leaves every forward-declared struct
		// as its empty 1-byte placeholder instead of filling it in with the later definition
		CParser parser = new CParser(dtm, true, null);
		int parsed = 0, rejected = 0;
		for (String block : blocks) {
			try {
				parser.parse(block);
				parsed++;
			}
			catch (Exception e) {
				if (rejected++ < 30) {
					println("rejected: " + block.substring(0, Math.min(block.length(), 70)).replace("\n", " "));
				}
			}
		}
		println("parsed " + parsed + " type blocks, rejected " + rejected);

		if (args.length > 2) {
			applyGlobals(dtm, Path.of(args[2]));
		}

		Map<String, String> map = new HashMap<>();
		for (String line : Files.readAllLines(Path.of(args[1]))) {
			String[] f = line.split("\t");
			if (f.length == 2) {
				map.put(f[0], f[1]);
			}
		}

		DecompInterface decomp = new DecompInterface();
		decomp.openProgram(currentProgram);
		int typed = 0, noParams = 0, noStruct = 0, failed = 0;
		for (Function f : currentProgram.getFunctionManager().getFunctions(true)) {
			String sname = map.get(f.getName());
			if (sname == null) {
				continue;
			}
			DataType st = findStruct(dtm, sname);
			if (st == null) {
				noStruct++;
				continue;
			}
			try {
				DecompileResults res = decomp.decompileFunction(f, 30, monitor);
				HighFunction high = res.getHighFunction();
				if (high == null) {
					failed++;
					continue;
				}
				HighFunctionDBUtil.commitParamsToDatabase(high, true,
					HighFunctionDBUtil.ReturnCommitOption.NO_COMMIT, SourceType.USER_DEFINED);
				Parameter[] params = f.getParameters();
				if (params.length == 0) {
					noParams++;
					continue;
				}
				params[0].setDataType(new PointerDataType(st), SourceType.USER_DEFINED);
				params[0].setName("this", SourceType.USER_DEFINED);
				typed++;
			}
			catch (Exception e) {
				failed++;
			}
		}
		decomp.dispose();
		println("typed " + typed + " methods; no params " + noParams + "; struct missing " + noStruct +
			"; failed " + failed + "; ambiguous struct names " + ambiguous);
	}

	// a name can resolve to several types (an empty forward declaration, a program type of the
	// same name, ...): take the largest defined one, never an empty placeholder
	private DataType findStruct(DataTypeManager dtm, String name) {
		return structs.computeIfAbsent(name, n -> {
			List<DataType> found = new ArrayList<>();
			dtm.findDataTypes(n, found);
			DataType best = null;
			for (DataType dt : found) {
				if (!dt.isNotYetDefined() && (best == null || dt.getLength() > best.getLength())) {
					best = dt;
				}
			}
			if (found.size() > 1 && ambiguous++ < 5) {
				StringBuilder sb = new StringBuilder("ambiguous " + n + ":");
				for (DataType dt : found) {
					sb.append(" " + dt.getPathName() + "/" + dt.getLength());
				}
				println(sb.toString());
			}
			return best;
		});
	}

	private static DataType primitive(String name) {
		switch (name) {
			case "f32": case "float": return FloatDataType.dataType;
			case "f64": case "double": return DoubleDataType.dataType;
			case "s8": return SignedCharDataType.dataType;
			case "u8": return UnsignedCharDataType.dataType;
			case "char": return CharDataType.dataType;
			case "s16": return ShortDataType.dataType;
			case "u16": return UnsignedShortDataType.dataType;
			case "s32": case "int": case "BOOL": return IntegerDataType.dataType;
			case "u32": return UnsignedIntegerDataType.dataType;
			case "bool": return BooleanDataType.dataType;
			case "wchar_t": return WideChar16DataType.dataType;
			case "void": return VoidDataType.dataType;
			default: return null;
		}
	}

	// rows: name, base type, pointer depth, element count
	private void applyGlobals(DataTypeManager dtm, Path path) throws Exception {
		Listing listing = currentProgram.getListing();
		int applied = 0, noSymbol = 0, noType = 0, failed = 0;
		for (String line : Files.readAllLines(path)) {
			String[] f = line.split("\t");
			if (f.length != 4) {
				continue;
			}
			DataType dt = primitive(f[1]);
			if (dt == null) {
				dt = findStruct(dtm, f[1]);
			}
			int ptr = Integer.parseInt(f[2]);
			int count = Integer.parseInt(f[3]);
			if (dt == null || (dt instanceof VoidDataType && ptr == 0)) {
				noType++;
				continue;
			}
			for (int i = 0; i < ptr; i++) {
				dt = new PointerDataType(dt, 4, dtm);
			}
			if (count > 1) {
				dt = new ArrayDataType(dt, count, dt.getLength(), dtm);
			}
			List<Symbol> syms = currentProgram.getSymbolTable().getGlobalSymbols(f[0]);
			if (syms.isEmpty()) {
				noSymbol++;
				continue;
			}
			Address a = syms.get(0).getAddress();
			try {
				listing.clearCodeUnits(a, a.add(dt.getLength() - 1), false);
				listing.createData(a, dt);
				applied++;
			}
			catch (Exception e) {
				failed++;
			}
		}
		println("typed " + applied + " globals; no symbol " + noSymbol + "; no type " + noType + "; failed " +
			failed);
	}
}
