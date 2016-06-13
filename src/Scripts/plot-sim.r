library("ggplot2")
library("reshape2")
setwd("~/dev/BitFunnel/src/Scripts")

png(filename="wat.png",width=800,height=600)
df <- read.csv(file="wat.csv")
munged <- melt(df)
ggplot(munged, aes(x=variable, y=value)) + geom_point()

dev.off()