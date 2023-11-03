#!/usr/bin/env python3

# Uses the Gitea API to fetch the latest revision of the project from a repo.
#
# Inspired by:
# https://github.com/olivergregorius/micropython_ota
#
# ----------------------------------------------------------------------------
# Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# See <http://www.gnu.org/licenses/>.
# ----------------------------------------------------------------------------

import util
import sys
import os

class StateUpdate:
    def __init__(self, lcd):
        self.host = "https://git.xythobuz.de"
        self.repo = "thomas/sb-py"
        self.branch = None
        self.exe_path = "states.py"
        self.blacklist = [
            "README.md",
            "COPYING",
            ".gitignore",
            "python-test/.gitignore",
            "python-test/copy.sh",
            "web-app/fetch.sh",
        ]

        self.lcd = lcd

        self.get = None
        self.version_file = "_ota_version"

    def fetch(self, url):
        # lazily initialize WiFi
        if self.get == None:
            self.get, post = util.getRequests()
            if self.get == None:
                return None

        try:
            #print("GET " + url)
            r = self.get(url)

            # explitic close on Response object not needed,
            # handled internally by r.content / r.text / r.json()
            # to avoid this automatic behaviour, first access r.content
            # to trigger caching it in response object, then close
            # socket.
            tmp = r.content
            if hasattr(r, "raw"):
                if r.raw != None:
                    r.raw.close()
                    r.raw = None

            return r
        except Exception as e:
            print()
            print(url)
            if hasattr(sys, "print_exception"):
                sys.print_exception(e)
            else:
                print(e)
            print()
            return None

    def get_stored_commit(self):
        current = "unknown"
        try:
            f = open(self.version_file, "r")
            current = f.readline().strip()
            f.close()
        except Exception as e:
            print()
            if hasattr(sys, "print_exception"):
                sys.print_exception(e)
            else:
                print(e)
            print()
        return current

    def get_previous_commit(self, commit):
        r = self.fetch(self.host + "/" + self.repo + "/commit/" + commit).text
        for line in r.splitlines():
            if not (self.repo + "/commit/") in line:
                continue

            line = line[line.find("/commit/") : ][8 : ][ : 40]
            if line != commit:
                return line
        return "unknown"

    def check(self, verbose = False):
        if self.branch == None:
            # get default branch
            r = self.fetch(self.host + "/api/v1/repos/" + self.repo).json()
            self.branch = r["default_branch"]

            if verbose:
                print("Selected default branch " + self.branch)

        # check for latest commit in branch
        r = self.fetch(self.host + "/api/v1/repos/" + self.repo + "/branches/" + self.branch).json()
        commit = r["commit"]["id"]

        if verbose:
            print("Latest commit is " + commit)

        current = self.get_stored_commit()

        if verbose:
            if current != commit:
                print("Current commit " + current + " is different!")
            else:
                print("No update required")

        return (current != commit, commit)

    def update_to_commit(self, commit, verbose = False):
        # list all files for a commit
        r = self.fetch(self.host + "/api/v1/repos/" + self.repo + "/git/trees/" + commit).json()

        # TODO does not support sub-folders

        if verbose:
            if len(r["tree"]) > 0:
                print(str(len(r["tree"])) + " files in repo:")
                for f in r["tree"]:
                    if f["path"] in self.blacklist:
                        print("  - (IGNORED) " + f["path"])
                    else:
                        print("  - " + f["path"])
            else:
                print("No files in repo?!")

        for f in r["tree"]:
            if f["path"] in self.blacklist:
                continue

            # get a file from a commit
            r = self.fetch(self.host + "/" + self.repo + "/raw/commit/" + commit + "/" + f["path"]).text

            if verbose:
                print("Writing " + f["path"])

            # overwrite existing file
            fo = open(f["path"], "w")
            fo.write(r)
            fo.close()

            if f["path"] == self.exe_path:
                if verbose:
                    print("Writing " + f["path"] + " to main.py")

                fo = open("./main.py", "w")
                fo.write(r)
                fo.close()

        # Write new commit id to local file
        f = open(self.version_file, "w")
        f.write(commit + "\n")
        f.close()

def pico_ota_run():
    print("Checking for updates")
    newer, commit = ota.check(True)

    if newer:
        print("Updating to:", commit)
        ota.update_to_commit(commit, True)

        print("Resetting")
        machine.soft_reset()
