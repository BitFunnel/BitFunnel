// Helper script to convert from Windows to Linux.

package main

import (
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"
)

func convertFile(path string) {
	input, err := ioutil.ReadFile(path)
	if err != nil {
		log.Fatal(err)
	}

	outLines := make([]string, 0)
	lastWasPragma := false
	inLines := strings.Split(string(input), "\n")
	for _, line := range inLines {
		// TODO: handle already ifdef'd out pragma.
		// #ifdef out VC++ pragmas.
		isPragma := strings.HasPrefix(
			strings.Trim(line, " "),
			"#pragma warning")

		if isPragma && !lastWasPragma {
			outLines = append(outLines, "#ifdef BITFUNNEL_PLATFORM_WINDOWS")
		}
		if !isPragma && lastWasPragma {
			outLines = append(outLines, "#endif // BITFUNNEL_PLATFORM_WINDOWS")
		}
		outLines = append(outLines, line)

		lastWasPragma = isPragma
	}

	// TODO: add check for missing `template <>`

	output := strings.Join(outLines, "\n")
	err = ioutil.WriteFile(path, []byte(output), 0644)
	if err != nil {
		log.Fatal(err)
	}
}

func convert(path string, info os.FileInfo, err error) error {
	if err != nil {
		log.Fatal(err)
		return nil
	}

	if info.IsDir() {
		return nil
	}

	if !(strings.HasSuffix(path, ".cpp") ||
		strings.HasSuffix(path, ".h")) {
		return nil
	}

	convertFile(path)
	return nil
}

func main() {
	// TODO: take parameter.
	err := filepath.Walk("/home/dluu/dev/BitFunnel/src/Common/CsvTsv", convert)
	if err != nil {
		log.Fatal(err)
	}
}
