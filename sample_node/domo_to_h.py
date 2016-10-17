import json
import requests

host = "localhost"
port = 8080

r = requests.get('http://{}:{}/json.htm?type=devices&filter=all'.format(host,port))
# data = json.loads(r.content.decode())
data = r.json()
# print(data)



# print(len(r['result']))
for i in data['result']:
	print("#define IDX_",end='')
	n = i['Name'].replace(' ','_')
	print(n, end=' ' )
	for c in range(30-len(n)):
		print(' ', end='')
	print(i['idx'])

print('')


r = requests.get('http://{}:{}/json.htm?type=command&param=getuservariables'.format(host,port))
data = r.json()
for i in data['result']:
	print("#define IDX_VAR_",end='')
	n = i['Name'].replace(' ','_')
	print(n, end=' ' )
	for c in range(26-len(n)):
		print(' ', end='')
	print(i['idx'])