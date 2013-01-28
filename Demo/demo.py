import serial
import time
import os





def SendCMD(cmd):
    tmp=port.read(port.inWaiting())
    tmp=port.write(cmd+'\r')
    tmp=''
    while ('>' not in tmp):
        tmp+=port.read(port.inWaiting())
    #tmp=port.read(port.inWaiting())
    #print tmp+'\n'


def SendStr(string):
    out_str='\t'.join(string.split(' '))
    cmd='w '+out_str
    SendCMD(cmd)

def chunks(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

def hextoint(val):
    return int(val,16)

port=serial.Serial('/dev/ttyACM0',115200)
f=file('LoremIpsum.txt','r')
Text=f.readlines()
f.close()
fps=[10,20,30,50,100]

SendCMD('c')
time.sleep(1)
print 'Show Splash Screen \n'
SendCMD('s')
time.sleep(2)

print 'Invert Colors \n'
SendCMD('i')
SendCMD('s')
time.sleep(2)
SendCMD('n')        



for freq in fps:
    print 'Animation, %i fps \n'%freq
    SendCMD('a %i'%freq)
    time.sleep(1)


print 'Single screen of Text \n'
SendCMD('c')
SendCMD('l 0')
for line in Text[:30]:
    SendStr(line.strip())

time.sleep(2)

print 'Scrolling Text \n'
SendCMD('c')
SendCMD('l 0')
for line in Text[:60]:
    SendStr(line.strip())

time.sleep(2)

print 'Single screen of inverted Text \n'
SendCMD('c')
SendCMD('i')
SendCMD('l 0')
for line in Text[:30]:
    SendStr(line.strip())
time.sleep(2)

print 'Bitmap Download'
files=os.listdir('.')
SendCMD('n')
SendCMD('c')
SendCMD('sp 0')
for name in files:
  if ('.c' in name):
    print name  
    f=file(name,'r')
    Data=f.readlines()
    f.close()
    OutData=[]
    for line in Data[6:-1]:
	tmp=line.strip().split(',')
	OutData+=tmp[:-1]

    OutData=map(hextoint,OutData)
    SendData=chunks(OutData,30)
    for line in SendData:
	Cmd='wp'
	for val in line:
	    Cmd+=' %i'%val
	SendCMD(Cmd)

time.sleep(2)
    


print 'Text on top of graphics'
SendCMD('n')
SendCMD('c')
SendCMD('s')
SendCMD('l 15')
SendStr(18*' '+"That's All Folks"+19*' ')
time.sleep(5)
SendCMD('c')

