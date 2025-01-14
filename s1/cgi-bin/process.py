#!/usr/bin/env python3

import cgi
import cgitb
import sys
import urllib.parse

cgitb.enable()  # Enable debugging for CGI scripts

# Read raw input
raw_input = sys.stdin.read()

# Parse input manually
form_data = urllib.parse.parse_qs(raw_input)
name = form_data.get('name', [''])[0]
email = form_data.get('email', [''])[0]
message = form_data.get('message', [''])[0]

# Save the data to a file
with open('s1/forms/submissions.txt', 'a') as f:
    f.write(f"Name: {name}\nEmail: {email}\nMessage: {message}\n---\n\n")

# Return an HTML response
print("<h1>Form Submitted Successfully</h1>")
print(f"<h2>Thank you, {name}!</h2>")
print('<a href="../home.html"><h2>Home</h2></a>')
