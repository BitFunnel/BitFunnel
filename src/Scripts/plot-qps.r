library("ggplot2")
setwd("~/dev/BitFunnel/src/Scripts")

args = commandArgs(trailingOnly=TRUE)
if (length(args) != 2) {
   stop("Required args: inputFilename, outputFilename", call.=FALSE)
}
inputName = args[1]
outputName = args[2]

print("Reading input")
df <- read.csv(header=TRUE, file=inputName)

print("Creating plot.")
png(filename=outputName,width=1600,height=1200)
ggplot(df, aes(x=Threads, y=QPS, fill=Treatment)) +
geom_bar(stat="identity", position="dodge") +
theme_minimal() +
scale_fill_brewer(palette="Set1") + 
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40),
      legend.title=element_text(size=40),
      legend.text=element_text(size=40))      
dev.off()