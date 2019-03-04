#/'/

from bottle import route, run, static_file, template, request
import granolapods
import time

@route('/', method=['GET','POST'])
def index():
	postdata = request.body.read()
	if len(postdata) > 0:
		pval = request.params.get('pval', type=int)
		fval = request.params.get('fval', type=int)
		cval = request.params.get('cval', type=int)
		granolapods.newRequestCallback(pval, fval, cval)
	return template('./htdocs/index.html')

@route('/static/<filename>')
def server_static(filename):
	return static_file(filename, root='./htdocs/')
	
if __name__ == '__main__':
	run(reLoader=True, debug=True)
