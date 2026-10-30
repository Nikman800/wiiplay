// Pre-analysis: seed r2 (_SDA2_BASE_) and r13 (_SDA_BASE_) across all code so
// small-data loads resolve to real addresses in the decompiler.
// Args: <sda_base> <sda2_base>
// @category WiiPlay
import ghidra.app.script.GhidraScript;
import ghidra.program.model.lang.Register;
import ghidra.program.model.listing.ProgramContext;
import ghidra.program.model.mem.MemoryBlock;

import java.math.BigInteger;

public class SetSdaRegisters extends GhidraScript {
	@Override
	protected void run() throws Exception {
		String[] args = getScriptArgs();
		BigInteger sda = new BigInteger(args[0].replaceFirst("^0x", ""), 16);
		BigInteger sda2 = new BigInteger(args[1].replaceFirst("^0x", ""), 16);
		ProgramContext ctx = currentProgram.getProgramContext();
		Register r13 = ctx.getRegister("r13");
		Register r2 = ctx.getRegister("r2");
		for (MemoryBlock block : currentProgram.getMemory().getBlocks()) {
			if (!block.isExecute()) {
				continue;
			}
			ctx.setValue(r13, block.getStart(), block.getEnd(), sda);
			ctx.setValue(r2, block.getStart(), block.getEnd(), sda2);
			println("r13=" + sda.toString(16) + " r2=" + sda2.toString(16) + " over " + block.getName());
		}
	}
}
