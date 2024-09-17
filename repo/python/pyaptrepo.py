#!/bin/python
#Description: Generates/Lists GPG Keys
import gnupg
from pydpkg import Dpkg
import sys
import hashlib
import shutil
from datetime import datetime
import os
from http.server import HTTPServer, SimpleHTTPRequestHandler

homedirectory = None
gpg = None

def get_action_input():
    print ("I want to:")
    print ("1. List Keys")
    print ("2. Export Keys")
    print ("3. Generate Keys")
    print ("4. Generate Apt Repository in Current Directory")
    print ("5. Start Webserver/Repo In Current Directory")
    print ("6. Quit")
    return input ("Enter Choice: ")

def list_keys(choose, secret):
    #todo get username/home directory automatically
    keys = gpg.list_keys(secret)
    count = 1
    for key in keys:
        print(f"[{count}] {key['fingerprint']} {key['uids'][0]}")
        count = count + 1
    if choose:
        i = input("Select Key: ")
        return keys[int(i) - 1]

def export_keys():
    key = list_keys(True, True)
    outputfilename = input("Enter filename for exporting key: ")
    gpg.export_keys(key['keyid'], secret=False, armor=False, output=outputfilename)

def generate_keys():
    input_data = gpg.gen_key_input(key_type="RSA", key_length=1024)
    key = gpg.gen_key(input_data)

def pkgfile_write(pkgfile, dp, debfile):
    pkgfile.write(f"Package: {dp['Package']}\n")
    pkgfile.write(f"Version: {dp['Version']}\n")
    pkgfile.write(f"Architecture: {dp['Architecture']}\n")
    pkgfile.write(f"Maintainer: {{dp['Maintainer']}}\n")
    pkgfile.write(f"Installed-Size: {dp['Installed-Size']}\n")
    #pkgfile.write(f"Depends: {dp['Depends']}\n")
    pkgfile.write(f"Filename: {debfile}\n")
    pkgfile.write(f"Size: {os.path.getsize(debfile)}\n")
    pkgfile.write(f"MD5Sum: {gethash(debfile, 'md5')}\n")
    pkgfile.write(f"SHA1: {gethash(debfile, 'sha1')}\n")
    pkgfile.write(f"SHA256: {gethash(debfile, 'sha256')}\n")
    pkgfile.write(f"Section: {dp['Section']}\n")
    pkgfile.write(f"Priority: {dp['Priority']}\n")
    pkgfile.write(f"Homepage: {dp['Homepage']}\n")
    pkgfile.write(f"Description: {dp['Description']}\n\n")

def generate_package_file(path, debfiles):
    print("Generating Packages file")
    pkgfile = open("Packages", "w")
    for debfile in debfiles:
        dp = Dpkg(debfile)
        pkgfile_write(pkgfile, dp, debfile)
    pkgfile.close()
    #shutil.copyfile ("Packages", os.path.join(path, "Packages"))

def gethash(filepath, hashtype):
    if hashtype == "md5":
        hash = hashlib.md5()
    elif hashtype == "sha1":
        hash = hashlib.sha1()
    elif hashtype == "sha256":
        hash = hashlib.sha256()

    with open(filepath, "rb") as f:
        while chunk := f.read(8192):
            hash.update(chunk)
    return hash.hexdigest()

def filelength(filepath):
        return os.path.getsize(filepath)

def generate_release_file(path, files):
    print (" Generating Release File")
    relfile = open("Release", "w")
    relfile.write("Origin: Test Repository\n")
    relfile.write("Label: Test\n")
    relfile.write("Suite: test\n")
    relfile.write("Codename: test\n")
    relfile.write("Version: 1.0\n")
    relfile.write("Architectures: all\n")
    relfile.write("Components: maintest\n")
    relfile.write("Description: Test software repo\n")
    relfile.write(f"Date: {datetime.today().strftime('%a, %d %m %Y %H:%M:%S +0000')}\n")

    relfile.write("MD5Sum:\n")
    for file in files:
        filepath = os.path.join(path, file)
        filename = file[(file.rfind("/") + 1):]
        relfile.write(f" {gethash(filepath, 'md5')} {filelength(filepath)} {filename}\n")
    relfile.write("SHA1:\n")
    for file in files:
        filepath = os.path.join(path, file)
        filename = file[(file.rfind("/") + 1):]
        relfile.write(f" {gethash(filepath, 'sha1')} {filelength(filepath)} {filename}\n")
    relfile.write("SHA256:\n")
    for file in files:
        filepath = os.path.join(path, file)
        filename = file[(file.rfind("/") + 1):]
        relfile.write(f" {gethash(filepath, 'sha256')} {filelength(filepath)} {filename}\n")
    
    relfile.close();
    #shutil.copyfile("Release", os.path.join(path, "Release"))

def sign_file(filepath, outputfile):
    print (f" Signature File {outputfile}")
    key = list_keys(True, True)
    file = open(filepath, "rb")
    sig = gpg.sign_file(file, keyid=key['keyid'], detach=True, output=outputfile)
    file.close()

def clearsign_file(filepath, outputfile):
    print (f" Signature File {outputfile}")
    key = list_keys(True, True)
    shutil.copyfile(filepath, "InRelease")
    file = open(outputfile, "rb")
    sig = gpg.sign_file(file, keyid=key['keyid'], detach=False, output=outputfile)
    file.close()

def get_files(path):
    print (" Scanning For Files\n");
    files = []
    for (dirpath, dirnames, filenames) in os.walk (path):
        for filename in filenames:
            files.append(dirpath + "/" + filename)
    return files

def generate_apt_repository():
    removefiles = ["Packages", "Release", "Release.gpg", "InRelease"]
    pkgdir = os.path.curdir #input("Enter Package Directory [Enter for .]: ")
    if pkgdir == "":
        pkgdir = os.path.curdir
        if os.access(pkgdir, os.W_OK):
            for removefile in removefiles:
                os.remove(os.path.join(pkgdir, removefile))
        else:
            print ("Package Directory Not Writable")
            return
   #get filenames from directory. Do Not iterate into directory
    files = get_files(pkgdir)
    
    #extract names of deb files from file list
    debfiles = [x for x in files if x[-4:]==".deb"]
    
    #Generate Packages file
    generate_package_file(pkgdir, debfiles)
    files.append(os.path.join(pkgdir,"Packages"))

    #Generate Release file
    generate_release_file(pkgdir, files)
    
    #Sign Release file. Generate Release.gpg
    sign_file(os.path.join(pkgdir, "Release"), "Release.gpg")
    clearsign_file(os.path.join(pkgdir, "Release"), "InRelease")
    #shutil.copyfile("Release.gpg", os.path.join(pkgdir, "Release.gpg"))

def start_webserver():
    print("Starting Webserver on port 8080")
    print("Press Ctrl+C to terminate")
    httpd = HTTPServer(("0.0.0.0",8080), RequestHandlerClass=SimpleHTTPRequestHandler)
    httpd.serve_forever()
    

# Begin Here
if os.environ.get("USER") == "root":
    print ("Do not run this script as root")
    exit(1)

homedirectory = os.path.expanduser("~")
gpg = gnupg.GPG(gnupghome=os.path.join(homedirectory,'.gnupg'))

while True:
    input_choice = get_action_input()
    if input_choice == "1":
        list_keys(False, True)
    elif input_choice == "2":
        export_keys()
    elif input_choice == "3":
        generate_keys()
        list_keys(False, True)
    elif input_choice == "4":
        generate_apt_repository()
    elif input_choice == "5":
        start_webserver()
    else:
        break
#TODO:
# set date in release file

