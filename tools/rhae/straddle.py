"""Give our data symbols that straddle a split boundary the boundaries of Rhae's symbols (-w writes)."""
import os,re,sys
RS=os.environ['RHAE']+'/config/RHAE01_01/'
L=re.compile(r'^(\S+) = (\.\w+):0x([0-9A-Fa-f]+); // (.*)$')
def syms(p):
    out=[]
    for l in open(p):
        m=L.match(l)
        if m:
            sz=re.search(r'size:0x([0-9A-Fa-f]+)',m.group(4)); out.append((m.group(2),int(m.group(3),16),int(sz.group(1),16) if sz else 0,m.group(1),l.rstrip('\n')))
    return out
ours=syms('config/RHAE01/symbols.txt'); theirs=syms(RS+'symbols.txt')
S=re.compile(r'\s+(\.\w+)\s+start:0x([0-9A-Fa-f]+)\s+end:0x([0-9A-Fa-f]+)')
bounds=set()
for l in open('config/RHAE01/splits.txt'):
    m=S.match(l)
    if m: bounds.add((m.group(1),int(m.group(2),16))); bounds.add((m.group(1),int(m.group(3),16)))
bad=[]
for sec,a,sz,n,l in ours:
    if sz and any((sec,x) in bounds for x in range(a+1,a+sz)): bad.append((sec,a,sz,n))
for b in bad: print(b[0],hex(b[1]),hex(b[2]),b[3])
print(len(bad),'straddling symbols')
if '-w' in sys.argv:
    drop=set(); add=[]; miss=[]
    for sec,a,sz,n in bad:
        rep=[t for t in theirs if t[0]==sec and a<=t[1]<a+sz]
        if not rep or rep[0][1]!=a or sum(t[2] for t in rep)>sz and rep[-1][1]+rep[-1][2]>a+sz: miss.append(n); continue
        drop.add((sec,a)); add+=rep
    by={(t[0],t[1]):t for t in add}
    out=[]
    for l in open('config/RHAE01/symbols.txt'):
        m=L.match(l)
        if m and (m.group(2),int(m.group(3),16)) in drop:
            k=(m.group(2),int(m.group(3),16))
            out+= [t[4]+'\n' for t in add if (t[0],t[1])==k or (t[0]==k[0] and k[1]<t[1]<k[1]+[b for b in bad if (b[0],b[1])==k][0][2])]
            continue
        out.append(l)
    open('config/RHAE01/symbols.txt','w').writelines(out)
    print('replaced',len(drop),'unresolved',miss)
