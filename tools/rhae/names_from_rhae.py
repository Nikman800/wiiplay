"""Give our generic data labels (lbl_*) inside the data ranges of the units Rhae matches (RHAE_LIBS)
the names Rhae's symbols.txt has at the same address and size (-w writes)."""
import os,re,sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from rhae_libs import RHAE_LIBS
LIBRE="|".join(re.escape(p) for p in RHAE_LIBS)
RR=os.environ['RHAE']+'/'
L=re.compile(r'^(\S+) = (\.\w+):0x([0-9A-Fa-f]+); // (.*)$')
S=re.compile(r'\s+(\.\w+)\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)')
cfg=open(RR+'configure.py').read()
M=set(re.findall(r'Object\(\s*Matching\s*,\s*"((?:'+LIBRE+r')[^"]+)"',cfg))
rng=[];cur=None
for l in open('config/RHAE01/splits.txt'):
    m=re.match(r'^(?!Sections:)(\S.*):\s*$',l)
    if m: cur=m.group(1); continue
    m=S.match(l)
    if m and cur in M and m.group(1)!='.text': rng.append((m.group(1),int(m.group(2),16),int(m.group(3),16)))
size=lambda c:(re.search(r'size:0x([0-9A-Fa-f]+)',c) or [0,'0'])[1]
theirs={}
for l in open(RR+'config/RHAE01_01/symbols.txt'):
    m=L.match(l)
    if m: theirs[(m.group(2),int(m.group(3),16))]=(m.group(1),size(m.group(4)),m.group(4))
ours=set(m.group(1) for l in open('config/RHAE01/symbols.txt') if (m:=L.match(l)))
ourrows=[m for l in open('config/RHAE01/symbols.txt') if (m:=L.match(l))]
# labels a source file still declares by name (extern "C" stand-ins) keep it until that source changes
import glob
srcrefs=set()
for f in glob.glob('src/**/*.c*',recursive=True)+glob.glob('include/**/*.h',recursive=True):
    srcrefs.update(re.findall(r'\blbl_[0-9A-F]{8}\b',open(f,errors='replace').read()))
out=[];n=0;skip=[];drop=set()
for l in open('config/RHAE01/symbols.txt'):
    m=L.match(l)
    if m and m.group(1).startswith('lbl_') and m.group(1) not in srcrefs:
        k=(m.group(2),int(m.group(3),16)); t=theirs.get(k)
        if t and not t[0].startswith(('lbl_','gap_')) and any(s==k[0] and a<=k[1]<e for s,a,e in rng):
            if t[1]!=size(m.group(4)):
                # a label over several of Rhae's objects: take them when they tile it exactly
                end=k[1]+int(size(m.group(4)),16); parts=[]; a=k[1]
                while a<end and (k[0],a) in theirs and int(theirs[(k[0],a)][1],16):
                    parts.append((a,)+theirs[(k[0],a)]); a+=int(theirs[(k[0],a)][1],16)
                tail=not any(k[0]==sec and a<=x<end for sec,x in theirs)  # only padding after the parts
                big=int(t[1],16)>int(size(m.group(4)),16)
                if big:
                    # one of Rhae's objects over several of our labels: take it when only labels lie under it
                    under=[(int(x.group(3),16),x.group(1)) for x in ourrows if x.group(2)==k[0] and k[1]<=int(x.group(3),16)<k[1]+int(t[1],16)]
                    if t[0] in ours or not all(nm.startswith('lbl_') for _,nm in under):
                        skip.append((m.group(1),t[0],'size')); out.append(l); continue
                    drop.update(nm for _,nm in under if nm!=m.group(1))
                    out.append(f'{t[0]} = {k[0]}:0x{k[1]:08X}; // {re.sub(r" *//.*","",t[2])}\n'); n+=1
                    continue
                if not parts or (a!=end and not (a<end and tail)) or (len(parts)<2 and a==end) or any(p[1] in ours and 'scope:local' not in p[3] for p in parts):
                    skip.append((m.group(1),t[0],'size')); out.append(l); continue
                for a,nm,sz,c in parts:
                    out.append(f'{nm} = {k[0]}:0x{a:08X}; // {re.sub(r" *//.*","",c)}\n'); n+=1
                continue
            if t[0] in ours and 'scope:local' not in t[2]: skip.append((m.group(1),t[0],'dup')); out.append(l); continue
            scope=' scope:local' if ('scope:local' in t[2] or re.fullmatch(r'@\d+',t[0])) and 'scope:' not in m.group(4) else ''
            l=f'{t[0]} = {k[0]}:0x{k[1]:08X}; // {m.group(4)}{scope}\n'; n+=1
    out.append(l)
print('renamed',n,'skipped',len(skip))
for s in skip[:20]: print('  skip',*s)
out=[l for l in out if not ((m:=L.match(l)) and m.group(1) in drop)]
if '-w' in sys.argv: open('config/RHAE01/symbols.txt','w').writelines(out)
