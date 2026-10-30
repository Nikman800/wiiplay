"""Replace our blocks of RHAE_LIBS (outside game code) with Rhae's for the units Rhae matches; report overlaps (-w writes)."""
import os,re,sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from rhae_libs import RHAE_LIBS
LIBRE="|".join(re.escape(p) for p in RHAE_LIBS)
R=os.environ['RHAE']+'/config/RHAE01_01/splits.txt'
O='config/RHAE01/splits.txt'
H=re.compile(r'^(?!Sections:)(\S.*):\s*$')
S=re.compile(r'\s+(\.\w+)\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)')
def blocks(lines):
    pre,bl,cur=[],[],None
    for l in lines:
        m=H.match(l)
        if m and not l.startswith('//'): cur=[m.group(1),[l]]; bl.append(cur); continue
        if cur is None: pre.append(l)
        elif l.strip(): cur[1].append(l)
    return pre,bl
pre,ours=blocks(open(O).read().splitlines())
_,theirs=blocks(open(R).read().splitlines())
cfg=open(os.environ['RHAE']+'/configure.py').read()
M=set(re.findall(r'Object\(\s*Matching\s*,\s*"((?:'+LIBRE+r')[^"]+)"',cfg))
tb={b[0]:b for b in theirs}
ob={b[0] for b in ours}
# Rhae's block for the units Rhae matches; ours for the rest (Rhae has no data ranges there)
def text0(b):
    for l in b[1]:
        m=S.match(l)
        if m and m.group(1)=='.text': return int(m.group(2),16)
    return 0
# library units inside game code keep our blocks (their data is laid out with the game units)
M={u for u in M if u in tb and not 0x80182E90<=text0(tb[u])<0x802E9CC0
   and not any('rename:' in l for l in tb[u][1])}  # our dtk has no section renames
out=[tb[b[0]] if b[0] in M else b for b in ours]
# Rhae units we lack go right after the Rhae unit before them
names=[b[0] for b in out]
for i,b in enumerate(theirs):
    if b[0] in M and b[0] not in names:
        prev=[t[0] for t in theirs[:i] if t[0] in names]
        k=names.index(prev[-1])+1 if prev else 0
        out.insert(k,b); names.insert(k,b[0]); print('insert',b[0],'after',prev[-1] if prev else None)
rn=[b for b in out if b[0].startswith(RHAE_LIBS)]
def rng0(b):
    for l in b[1]:
        m=S.match(l)
        if m: yield m.group(1),int(m.group(2),16),int(m.group(3),16)
taken=[r for b in out if b[0] in M for r in rng0(b)]
def clash(l):
    m=S.match(l)
    return m and any(s==m.group(1) and a<int(m.group(3),16) and int(m.group(2),16)<e for s,a,e in taken)
new=[]
for b in out:
    if b[0] in M or not any(clash(l) for l in b[1]): new.append(b); continue
    if any(clash(l) and l.split()[0]=='.text' for l in b[1]): print('drop block',b[0]); continue
    print('trim',b[0],[l.strip() for l in b[1] if clash(l)]); new.append([b[0],[l for l in b[1] if not clash(l)]])
out=new
def rng(b):
    for l in b[1]:
        m=S.match(l)
        if m: yield m.group(1),int(m.group(2),16),int(m.group(3),16)
allr=sorted((s,a,e,b[0]) for b in out for s,a,e in rng(b))
bad=0
for i in range(1,len(allr)):
    p,c=allr[i-1],allr[i]
    if p[0]==c[0] and c[1]<p[2]: print('OVERLAP',p,c); bad+=1
if '-w' in sys.argv and not bad:
    txt='\n'.join(pre).rstrip('\n')+'\n\n'+'\n\n'.join('\n'.join(b[1]) for b in out)+'\n'
    open(O,'w',newline='\n').write(txt); print('written',len(rn),'library blocks')
print('overlaps',bad)
