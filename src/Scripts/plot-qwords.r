library("ggplot2")
library("reshape2")
setwd("~/dev/BitFunnel/src/Scripts")

# See
# https://www.r-bloggers.com/choosing-colour-palettes-part-ii-educated-choices/
# for color information.

args = commandArgs(trailingOnly=TRUE)
if (length(args) != 3) {
   stop("Required args: inputFilename, outputFilename, outputFilename", call.=FALSE)
}
inputName = args[1]
output0Name = args[2]
output1Name = args[3]

print("Reading input")
queries <- read.csv(header=TRUE, file=inputName)

# Create dataframe in fully normalized form.
df <- melt(queries, measure.vars=c("Quadwords","Cachelines","ExpectedCachelines"), id.vars="TermPos")

# Rename meaningless column name.
names(df)[names(df) == 'variable'] <- 'AccessType'

print("Creating plot.")
png(filename=output0Name,width=1600,height=1200)
ggplot(df, aes(x=TermPos,y=value,colour=AccessType)) +
scale_fill_brewer(palette="Set1") + # doesn't work :-(
theme_minimal() +
geom_point(alpha=1/150) +
guides(colour = guide_legend(override.aes = list(alpha = 1))) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40),
      legend.title=element_text(size=40),
      legend.text=element_text(size=40))
dev.off()

queries$Quadwords <- queries$Quadwords / 8
df <- melt(queries, measure.vars=c("Quadwords","Cachelines","ExpectedCachelines"), id.vars="TermPos")
names(df)[names(df) == 'variable'] <- 'AccessType'

print("Creating plot.")
png(filename=output1Name,width=1600,height=1200)
ggplot(df, aes(x=TermPos,y=value,colour=AccessType)) +
scale_fill_brewer("Set1") + # doesn't work :-(
theme_minimal() +
geom_point(alpha=1/150) +
guides(colour = guide_legend(override.aes = list(alpha = 1))) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40),
      legend.title=element_text(size=40),
      legend.text=element_text(size=40))
dev.off()
