"""Data Rhae marks scope:weak (string pools and statics of inline functions, which every
translation unit using the inline emits) takes Rhae's name and weak scope in our symbols.txt,
so a source-linked unit's copy merges with the asm one instead of being kept twice (-w writes)."""
import os,re,sys
RR=os.environ['RHAE']+'/'
L=re.compile(r'^(\S+) = (\.\w+):0x([0-9A-Fa-f]+); // (.*)$')
size=lambda c:(re.search(r'size:0x([0-9A-Fa-f]+)',c) or [0,'0'])[1]
weak={(m.group(2),m.group(3)):(m.group(1),size(m.group(4))) for l in open(RR+'config/RHAE01_01/symbols.txt')
      if (m:=L.match(l)) and 'scope:weak' in m.group(4) and m.group(2)!='.text'}
lines=open('config/RHAE01/symbols.txt').readlines()
names={m.group(1) for l in lines if (m:=L.match(l))}
n=0;skip=[]
for i,l in enumerate(lines):
    m=L.match(l)
    if not m or (m.group(2),m.group(3)) not in weak: continue
    nm,sz=weak[(m.group(2),m.group(3))]
    if sz!=size(m.group(4)): skip.append((m.group(1),nm,'size')); continue
    if nm!=m.group(1) and not m.group(1).startswith('lbl_'): skip.append((m.group(1),nm,'named')); continue
    if nm!=m.group(1) and nm in names: skip.append((m.group(1),nm,'name taken')); continue
    c=re.sub(r' scope:\w+','',m.group(4)); c=re.sub(r'(size:0x[0-9A-Fa-f]+)',r'\1 scope:weak',c,count=1)
    new=f'{nm} = {m.group(2)}:0x{m.group(3)}; // {c}\n'
    if new!=l: lines[i]=new; n+=1
print('weak',n,'skipped',len(skip))
for s in skip: print('  skip',*s)
if '-w' in sys.argv: open('config/RHAE01/symbols.txt','w').writelines(lines)
