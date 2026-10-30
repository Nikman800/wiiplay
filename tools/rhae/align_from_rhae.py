"""Copy Rhae's align: hints onto our symbols inside Rhae-library data ranges."""
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
for l in open(RR+'config/RHAE01_01/splits.txt'):
    m=re.match(r'^(?!Sections:)(\S.*):\s*$',l)
    if m: cur=m.group(1); continue
    m=S.match(l)
    if m and cur and cur.startswith(RHAE_LIBS) and m.group(1)!=".text": rng.append((m.group(1),int(m.group(2),16),int(m.group(3),16)))
al={}
for l in open(RR+'config/RHAE01_01/symbols.txt'):
    m=L.match(l)
    if m:
        a=re.search(r'\balign:(\d+)',m.group(4))
        if a: al[(m.group(2),int(m.group(3),16))]=a.group(1)
out=[];n=0
for l in open('config/RHAE01/symbols.txt'):
    m=L.match(l)
    if m:
        k=(m.group(2),int(m.group(3),16))
        if k in al and any(s==k[0] and a<=k[1]<e for s,a,e in rng):
            want='align:'+al[k]
            if re.search(r'\balign:\d+',l):
                if want not in l: l=re.sub(r'\balign:\d+',want,l); n+=1
            else:
                l=re.sub(r'(size:0x[0-9A-Fa-f]+)',r'\1 '+want,l.rstrip('\n'),1)+'\n'; n+=1
    out.append(l)
open('config/RHAE01/symbols.txt','w').writelines(out); print('aligned',n)
