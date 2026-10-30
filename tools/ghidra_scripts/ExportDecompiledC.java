// Post-analysis: decompile every function and write one .c file per
// decomp-toolkit unit, plus an index of every function.
// Args: <out_dir> <units.tsv>   (units.tsv: name<TAB>start_hex<TAB>end_hex)
// @category WiiPlay
import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileOptions;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.decompiler.parallel.DecompileConfigurer;
import ghidra.app.decompiler.parallel.DecompilerCallback;
import ghidra.app.decompiler.parallel.ParallelDecompiler;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.util.task.TaskMonitor;

import java.io.PrintWriter;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

public class ExportDecompiledC extends GhidraScript {

	private record Unit(String name, long start, long end) {}

	private record Output(Function function, String code, String error) {}

	@Override
	protected void run() throws Exception {
		String[] args = getScriptArgs();
		Path outDir = Path.of(args[0]);
		Files.createDirectories(outDir);
		List<Unit> units = new ArrayList<>();
		for (String line : Files.readAllLines(Path.of(args[1]))) {
			String[] f = line.split("\t");
			if (f.length == 3) {
				units.add(new Unit(f[0], Long.parseLong(f[1], 16), Long.parseLong(f[2], 16)));
			}
		}

		String prefix = args.length > 2 ? args[2] : "";  // optional function-name filter
		List<Function> functions = new ArrayList<>();
		currentProgram.getFunctionManager().getFunctions(true).forEach(f -> {
			if (f.getName().startsWith(prefix)) {
				functions.add(f);
			}
		});
		println("Decompiling " + functions.size() + " functions");

		DecompileConfigurer configurer = decompiler -> {
			DecompileOptions options = new DecompileOptions();
			options.grabFromProgram(currentProgram);
			decompiler.setOptions(options);
			decompiler.toggleCCode(true);
			decompiler.toggleSyntaxTree(false);
			decompiler.setSimplificationStyle("decompile");
		};
		DecompilerCallback<Output> callback = new DecompilerCallback<>(currentProgram, configurer) {
			@Override
			public Output process(DecompileResults results, TaskMonitor m) {
				Function fn = results.getFunction();
				if (results.getDecompiledFunction() == null) {
					return new Output(fn, null, results.getErrorMessage());
				}
				return new Output(fn, results.getDecompiledFunction().getC(), null);
			}
		};
		callback.setTimeout(120);
		List<Output> outputs;
		try {
			outputs = ParallelDecompiler.decompileFunctions(callback, functions, monitor);
		}
		finally {
			callback.dispose();
		}

		// parallel decompilation finishes in arbitrary order; sort so output is reproducible
		outputs.removeIf(java.util.Objects::isNull);
		outputs.sort(java.util.Comparator.comparingLong(o -> o.function().getEntryPoint().getOffset()));
		Map<String, TreeMap<Long, Output>> byUnit = new TreeMap<>();
		int failed = 0;
		try (PrintWriter index = new PrintWriter(Files.newBufferedWriter(outDir.resolve("functions.tsv"), StandardCharsets.UTF_8))) {
			index.println("address\tsize\tname\tunit\tstatus");
			for (Output o : outputs) {
				if (o == null) {
					continue;
				}
				long addr = o.function().getEntryPoint().getOffset();
				String unit = "unassigned";
				for (Unit u : units) {
					if (addr >= u.start() && addr < u.end()) {
						unit = u.name();
						break;
					}
				}
				byUnit.computeIfAbsent(unit, k -> new TreeMap<>()).put(addr, o);
				if (o.code() == null) {
					failed++;
				}
				index.printf("%08X\t%d\t%s\t%s\t%s%n", addr, o.function().getBody().getNumAddresses(),
					o.function().getName(), unit, o.code() == null ? "error: " + o.error() : "ok");
			}
		}

		for (Map.Entry<String, TreeMap<Long, Output>> e : byUnit.entrySet()) {
			try (PrintWriter w = new PrintWriter(Files.newBufferedWriter(outDir.resolve(e.getKey() + ".c"), StandardCharsets.UTF_8))) {
				w.println("// Ghidra pseudocode for decomp-toolkit unit " + e.getKey());
				w.println("// Program: " + currentProgram.getName() + "  Language: " + currentProgram.getLanguageID());
				w.println();
				for (Output o : e.getValue().values()) {
					w.printf("// %s @ %s%n", o.function().getName(), o.function().getEntryPoint());
					if (o.code() == null) {
						w.println("// decompilation failed: " + o.error());
						w.println();
					}
					else {
						w.println(o.code());
					}
				}
			}
		}
		println("Wrote " + byUnit.size() + " files; " + failed + " functions failed");
	}
}
