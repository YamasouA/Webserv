import os

for env in envs:
	if os.getenv(env):
		val = os.getenv(env)
	else:
		val = ""
	print(env + "=" + val) 
