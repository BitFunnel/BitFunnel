package main

// Hack script to generate HTML version of StrangeLoop talk
//
// First, run convert -density 300 slides.pdf -resize 25% strangeloop.png

import (
	"fmt"
)

var (
	preText =
`<div class="slideplustext">
<div class="slide">
<img src="/strangeloop/strangeloop-`
	postText =
`.png"></a>
</div>
<div class="transcript">

</div>
<p></div></p>
`

)

func main() {
	for i := 0; i < 105; i++ {
		fmt.Print(preText)
		fmt.Print(i)
		fmt.Print(postText)
	}
}
