"""For split ranges preceded by a pure-padding gap, give the range's first symbol an align: hint."""
import os,re,sys
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
from rhae_libs import RHAE_LIBS
LIBRE="|".join(re.escape(p) for p in RHAE_LIBS)
L=re.compile(r'^(\S+) = (\.\w+):0x([0-9A-Fa-f]+); // (.*)$')
S=re.compile(r'\s+(\.\w+)\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)')
rs={};cur=None
for l in open('config/RHAE01/splits.txt'):
    m=re.match(r'^(?!Sections:)(\S.*):\s*$',l)
    if m: cur=m.group(1); continue
    m=S.match(l)
    if m and m.group(1) not in ('.text','.init','.ctors','.dtors','extab','extabindex'): rs.setdefault(m.group(1),[]).append((int(m.group(2),16),int(m.group(3),16),cur))
lines=open('config/RHAE01/symbols.txt').readlines()
syms={}
for i,l in enumerate(lines):
    m=L.match(l)
    if m: syms.setdefault(m.group(2),[]).append((int(m.group(3),16),i))
n=0
for sec,r in rs.items():
    r.sort(); addrs=sorted(syms.get(sec,[]))
    for (a1,e1,u1),(a2,e2,u2) in zip(r,r[1:]):
        gap=a2-e1
        if gap<=0 or not (u1.startswith(RHAE_LIBS) or u2.startswith(RHAE_LIBS)): continue
        if any(e1<=a<a2 for a,_ in addrs): continue   # objects in the gap: dtk splits them itself
        A=next((A for A in (4,8,16,32,64,128) if a2%A==0 and (e1+A-1)//A*A==a2),None)
        k=[i for a,i in addrs if a==a2]
        if not A or not k: print('cannot',sec,hex(e1),hex(a2),u2); continue
        l=lines[k[0]]; m=re.search(r'\balign:(\d+)',l)
        if m and int(m.group(1))>=A: continue
        lines[k[0]]=re.sub(r'\balign:\d+','align:%d'%A,l) if m else re.sub(r'(size:0x[0-9A-Fa-f]+)',r'\1 align:%d'%A,l,count=1)
        n+=1; print(sec,hex(e1),'->',hex(a2),'align',A,u2)
open('config/RHAE01/symbols.txt','w').writelines(lines); print('added',n)
