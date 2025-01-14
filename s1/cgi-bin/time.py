#!/usr/bin/env python3

import cgi
import cgitb
from datetime import datetime

cgitb.enable()

# Get the current time
current_time = datetime.now()

# Output the time
print(f"<H1>Current time using Python: {current_time.strftime('%d-%m-%Y %H:%M:%S')}</H1>")