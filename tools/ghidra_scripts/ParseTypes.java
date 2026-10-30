// Parse a generated C header with Ghidra's CParser, one struct definition at a time, and report
// every definition that fails (the headless log only says "Problem Parsing" for a whole file).
// Args: <header.h>
// @category WiiPlay
import ghidra.app.script.GhidraScript;
import ghidra.app.util.cparser.C.CParser;
import ghidra.program.model.data.StandAloneDataTypeManager;

import java.nio.file.Files;
import java.nio.file.Path;

public class ParseTypes extends GhidraScript {
	@Override
	protected void run() throws Exception {
		String src = Files.readString(Path.of(getScriptArgs()[0]));
		// blocks: the forward-declaration prefix, then one "struct X { ... };" per block
		String[] blocks = src.split("\n(?=(?:struct|union) \\w+ \\{)");
		StandAloneDataTypeManager dtm = new StandAloneDataTypeManager("probe");
		int tx = dtm.startTransaction("parse");
		int ok = 0, bad = 0;
		try {
			for (String block : blocks) {
				CParser parser = new CParser(dtm, true, null);
				try {
					parser.parse(block);
					ok++;
				}
				catch (Throwable e) {
					bad++;
					if (bad <= 10) {
						String head = block.length() > 300 ? block.substring(0, 300) : block;
						println("PARSE ERROR in block: " + head.replace("\n", " | "));
						println("  " + e.getMessage() + " / " + parser.getParseMessages());
					}
				}
			}
			println("PARSE DONE: " + ok + " blocks ok, " + bad + " failed");
		}
		finally {
			dtm.endTransaction(tx, false);
		}
	}
}
