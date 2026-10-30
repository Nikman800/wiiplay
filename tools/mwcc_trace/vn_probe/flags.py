import sys,re
sys.path.insert(0,os.path.join(os.getcwd(), 'tools'))
from cccmd import base_command
u=sys.argv[1]
c=base_command(u+'.cpp')
f=c.split('mwcceppc.exe',1)[1]
f=f.split(' -c ')[0].replace(' -MMD','')
import os
print(f+' -i src/'+os.path.dirname(u))
