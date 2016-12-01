library("ggplot2")
library("reshape2")
setwd("~/dev/BitFunnel/src/Scripts")

args = commandArgs(trailingOnly=TRUE)
if (length(args) != 3) {
   stop("Required args: [interpreter QueryPipelineStats filename], [compiler QPS filename], [output filename]", call.=FALSE)
}
int_name = args[1]
comp_name = args[2]
out_name = args[3]

print("Reading input.")
interpreter <- read.csv(header=TRUE, file=int_name)
compiler <- read.csv(header=TRUE, file=comp_name)

df <- data.frame(interpreter$cachelines, compiler$match)
names(df)[names(df) == 'interpreter.cachelines'] <- 'Cachelines'
names(df)[names(df) == 'compiler.match'] <- 'MatchTime'

print("Plotting.")
png(filename=out_name,width=1600,height=1200)
ggplot(df, aes(x=Cachelines,y=MatchTime)) +
theme_minimal() +
geom_point(alpha=1/50) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40)) +
ylim(0, 0.00004)
dev.off()
