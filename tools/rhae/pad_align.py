"""Inside Rhae-library data ranges, a symbol that follows padding (a gap no symbol covers) gets the
smallest align: hint that reproduces the padding, so asm units lay out like the original."""
import os,re,sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from rhae_libs import RHAE_LIBS
L=re.compile(r'^(\S+) = (\.\w+):0x([0-9A-Fa-f]+); // (.*)$')
S=re.compile(r'\s+(\.\w+)\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)')
rng=[];cur=None
for l in open('config/RHAE01/splits.txt'):
    m=re.match(r'^(?!Sections:)(\S.*):\s*$',l)
    if m: cur=m.group(1); continue
    m=S.match(l)
    if m and cur.startswith(RHAE_LIBS) and m.group(1) not in ('.text','.ctors','.dtors','extab','extabindex'):
        rng.append((m.group(1),int(m.group(2),16),int(m.group(3),16)))
lines=open('config/RHAE01/symbols.txt').readlines()
rows={}
for i,l in enumerate(lines):
    m=L.match(l)
    if m and 'type:label' not in m.group(4):
        sz=re.search(r'size:0x([0-9A-Fa-f]+)',m.group(4))
        rows.setdefault(m.group(2),[]).append((int(m.group(3),16),int(sz.group(1),16) if sz else 0,i))
n=0
for sec,a0,e0 in rng:
    r=sorted(x for x in rows.get(sec,[]) if a0<=x[0]<e0)
    for (p,ps,_),(a,_,i) in zip(r,r[1:]):
        end=p+ps
        if end>=a or ps==0: continue
        A=next((A for A in (4,8,16,32,64,128) if a%A==0 and (end+A-1)//A*A==a),None)
        if not A: continue
        l=lines[i]; m=re.search(r'\balign:(\d+)',l)
        if m and int(m.group(1))>=A: continue
        lines[i]=re.sub(r'\balign:\d+','align:%d'%A,l) if m else re.sub(r'(size:0x[0-9A-Fa-f]+)',r'\1 align:%d'%A,l,count=1)
        n+=1
open('config/RHAE01/symbols.txt','w').writelines(lines); print('align hints added',n)
