#!/bin/bash

mkdir GET_DENIED
mkdir POST_DENIED
mkdir POST
cp cgi_post.py ./POST/
mkdir CGI_DENIED
mkdir CGI
cp cgi.py ./CGI/
cp syntax_error_cgi.py ./CGI/
cp cgi.py ./CGI_DENIED/
cp cgi_post.py ./CGI_DENIED/
mkdir redirect
