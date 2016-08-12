library("ggplot2")
library("reshape2")
setwd("~/dev/BitFunnel/src/Scripts")

# png(filename="wat.png",width=800,height=600)
png(filename="talk.png",width=1600,height=1200)
# df <- read.csv(header=FALSE, file="wat.csv")
df <- read.csv(header=FALSE, file="talk.csv")
## df <- read.csv(file="wat.csv")
## munged <- melt(df)
## ggplot(munged, aes(x=variable, y=value)) + geom_point()

# plot single
# ggplot(data=df, aes(x=factor(V1), y=V2)) + geom_bar(stat="identity") + labs(x = "Number of cache misses", y = "count")

# plot uniform 20 vs. buggy distribution
# ggplot(data=df, aes(x=factor(V1), y=V2)) + geom_bar(stat="identity") + facet_wrap(~ V3, ncol=1) + labs(x = "Number of cache misses", y = "count")

# plot for talk
ggplot(data=df, aes(x=factor(V1), y=V2)) + geom_bar(stat="identity") + labs(x = "Memory accesses", y = "Count") +
  theme(axis.text.x = element_text(size=40))

dev.off()
