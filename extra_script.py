Import("env")

def before_upload():
    print("Running the magic script that turns the html into a header file")
    html_file = open("src/index.html", "r")
    header_file = open("include/index.h", "w")

    header_file.write('const char MAIN_page[] PROGMEM = R"=====(' + html_file.read() + ')=====";')

before_upload()