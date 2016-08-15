// Find dependencies of a file.
//
// Note that we can't use gcc or clang -MD for this because the code in question
// only compiles with an ancient version of Visual Studio.
//
// This is a terrible hack.

package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"regexp"
	"strings"
)

var (
	includeCapture = regexp.MustCompile(`^#include\s+"(\S+)"`)
)

func scanFile(fpath string, queue *[]string) {
	file, err := os.Open(fpath)
	if err != nil {
		log.Fatal("scanFile ", fpath, err)
	}
	defer file.Close()

	scanner := bufio.NewScanner(file)
	for scanner.Scan() {
		line := scanner.Text()
		capture := includeCapture.FindStringSubmatch(line)
		if len(capture) > 1 {
			fullPath := capture[1]
			headerName := filepath.Base(fullPath)
			rawName := strings.TrimSuffix(headerName, filepath.Ext(headerName))
			fmt.Println(fullPath)
			*queue = append(*queue, rawName)
		}
	}

	if err := scanner.Err(); err != nil {
		log.Fatal(fpath, err)
	}
}

func getFullPath(rawName string, basepath string) []string {
	args := []string{basepath, "-name", fmt.Sprintf("%s.*", rawName)}
	output, err := exec.Command("find", args...).Output()
	if err != nil {
		log.Fatal("getFullPath", rawName, err)
	}
	paths := strings.Split(string(output), "\n")
	for i, _ := range paths {
		paths[i] = strings.TrimSpace(paths[i])
	}
	return paths
}

// Takes a file name (no extension) and a base directory.  Returns the
// transitive closure of dependencies for the file.
//
// Looks at each files, finds both the .cpp and .h (.*) files, and then prints
// out their #includes. Adds includes to a queue, and then walks through each
// item in the queue and repeats until the queue is empty.
func main() {
	rawName := os.Args[1]
	basepath := os.Args[2]

	queue := make([]string,0)
	queue = append(queue, rawName)

	seen := map[string]struct{}{
		"stdafx" : struct{}{},
		"UnitTest" : struct{}{},
	}

	for len(queue) > 0 {
		rawName = queue[0]
		queue = queue[1:]
		if _, ok := seen[rawName]; !ok {
			seen[rawName] = struct{}{}

			fmt.Println("----",rawName)
			paths := getFullPath(rawName, basepath)
			for _, pp := range paths {
				if (pp != "") {
					scanFile(pp, &queue)
				}
			}
		}
	}
}
