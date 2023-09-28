#!/bin/bash

cd ../srcs
mkdir upload
cp ../test/index.html ./
touch no_content.html
mkdir GET_DENIED
mkdir POST_DENIED
mkdir POST
cp ../test/cgi_post.py ./POST/
cp ../test/syntax_error_cgi.py ./POST/
mkdir CGI_DENIED
mkdir CGI
cp ../test/cgi.py ./CGI/
cp ../test/cgi_only_header.py ./CGI/
cp ../test/cgi_time_out.py ./CGI/
cp ../test/syntax_error_cgi.py ./CGI/
cp ../test/cgi.py ./CGI_DENIED/
cp ../test/cgi_post.py ./CGI_DENIED/
mkdir redirect
touch permission.py
chmod 000 permission.py
