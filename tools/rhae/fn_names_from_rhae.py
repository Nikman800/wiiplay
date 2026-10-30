"""Functions inside the .text of the units Rhae matches (RHAE_LIBS) take Rhae's name where Rhae
has a function at the same address and size under another name (-w writes). Names a source file
(game code) still uses, and names ours keeps elsewhere, are reported and left alone."""
import glob,os,re,sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from rhae_libs import RHAE_LIBS
RR=os.environ['RHAE']+'/'
LIBRE="|".join(re.escape(p) for p in RHAE_LIBS)
L=re.compile(r'^(\S+) = \.text:0x([0-9A-Fa-f]+); // (.*)$')
S=re.compile(r'\s+\.text\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)')
M=set(re.findall(r'Object\(\s*Matching\s*,\s*"((?:'+LIBRE+r')[^"]+)"',open(RR+'configure.py').read()))
rng=[];cur=None
for l in open('config/RHAE01/splits.txt'):
    m=re.match(r'^(?!Sections:)(\S.*):\s*$',l)
    if m: cur=m.group(1); continue
    m=S.match(l)
    if m and cur in M: rng.append((int(m.group(1),16),int(m.group(2),16)))
size=lambda c:(re.search(r'size:0x([0-9A-Fa-f]+)',c) or [0,'0'])[1]
theirs={int(m.group(2),16):(m.group(1),size(m.group(3))) for l in open(RR+'config/RHAE01_01/symbols.txt') if (m:=L.match(l))}
lines=open('config/RHAE01/symbols.txt').readlines()
ours={m.group(1) for l in lines if (m:=re.match(r'^(\S+) = ',l))}
srcrefs=set()
for f in glob.glob('src/Pack/**/*.c*',recursive=True)+glob.glob('include/Pack/**/*.h',recursive=True):
    srcrefs.update(re.findall(r'[A-Za-z_][A-Za-z0-9_]*',open(f,errors='replace').read()))
# addresses dtk/name_corrections.tsv names on evidence of their own keep that name
fixed={int(l.split('\t')[0],16) for l in open('dtk/name_corrections.tsv').read().splitlines()[1:] if l.strip()}
n=0;skip=[];ren={}
for i,l in enumerate(lines):
    m=L.match(l)
    if not m: continue
    a=int(m.group(2),16); t=theirs.get(a)
    if not t or t[0]==m.group(1) or t[1]!=size(m.group(3)) or not any(s<=a<e for s,e in rng): continue
    if t[0].startswith(('fn_','lbl_')): continue
    if a in fixed: skip.append((m.group(1),t[0],'name_corrections.tsv')); continue
    if m.group(1) in srcrefs: skip.append((m.group(1),t[0],'used by game code')); continue
    ren[i]=(m.group(1),t[0])
# names renamed away are free (chains of names shifted by one function)
final=(ours-{o for o,_ in ren.values()})
for i,(o,nw) in sorted(ren.items()):
    if nw in final: skip.append((o,nw,'name taken')); continue
    lines[i]=lines[i].replace(o+' = ',nw+' = ',1); final.add(nw); n+=1
print('renamed',n,'skipped',len(skip))
for s in skip: print('  skip',*s)
if '-w' in sys.argv: open('config/RHAE01/symbols.txt','w').writelines(lines)
