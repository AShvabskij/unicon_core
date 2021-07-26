The folder to public files to frontend

Need to bundle all mjs with dependencies by browserify!
Use command  "browserify script_demo_model.js -o bundle.js -d" from "public" subfolder
or "browserify --debug script_demo_model.js -o bundle.js"
Use command browserify public\script_demo_model.js -o public\bundle.js -d from project root folder

To install browserfy:
npm install --global browserify
