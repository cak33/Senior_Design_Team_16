import sys

from interface import Interface

interface = Interface()

try:
	data = sys.argv[1]
	if data:
		print("PENIS")
		f = open('ipc_file.txt', 'w')
		f.write(data)
		f.close()
		interface.run('website')
except:
	print("ERROR")


