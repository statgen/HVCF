from flask import Flask, request
import re

app = Flask(__name__)

def parse_filter_argument(filter):
   operator_pattern = r"(eq|le|ge)"
   variable_name_pattern = r"([a-zA-Z][a-zA-Z0-9_-]*)"
   string_value_pattern = r"(?:'[^']*')"
   uint_value_pattern = r"(?:0|[1-9]\d*)"
   value_pattern = "".join(["(", string_value_pattern, "|", uint_value_pattern, ")"])
   and_pattern = r"\s+and\s+"
   operation_pattern = "".join([variable_name_pattern, "\s+", operator_pattern, "\s+", value_pattern]) 
   
   operation_regex = re.compile(operation_pattern)
   and_regex = re.compile(and_pattern)

   start = 0
   end = len(filter)

   while True:
      match = operation_regex.match(filter, start)
      if not match:
         print 'bad 1'
         return

      print match.groups()

      start = match.end()
      if start >= end:
         break

      match = and_regex.match(filter, start) 
      if not match:
         print 'bad2'
         return
      start = match.end()

@app.route('/statistic/pair/LD/', methods = ['GET'])
def get_available_datasets():
   pass

@app.route('/statistic/pair/LD/results', methods = ['GET'])
def get_ld():
   
   for arg in request.args:
      if arg == 'filter':
         parse_filter_argument(request.args[arg])


   return 'oops'

if __name__ == '__main__':
   app.run(host='0.0.0.0', port=5000)
