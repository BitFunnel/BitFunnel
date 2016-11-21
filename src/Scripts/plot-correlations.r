# Takes input from correlation-histogram.py

library("ggplot2")
setwd("~/dev/BitFunnel/src/Scripts")

png(filename="term-term.png",width=1600,height=1200)
# df <- read.csv(header=FALSE, file="wat.csv")
term_term <- read.csv(header=TRUE, file="/tmp/term-term.csv")

ggplot(data=term_term, aes(x=bucket, y=count, fill=treatment)) + geom_bar(stat="identity", position="dodge")  +
  scale_y_log10() +
  theme_bw() +
  theme(axis.text = element_text(size=40),
        axis.title = element_text(size=40))
dev.off()

png(filename="term-all.png",width=1600,height=1200)
# df <- read.csv(header=FALSE, file="wat.csv")
term_all <- read.csv(header=TRUE, file="/tmp/term-all.csv")

ggplot(data=term_all, aes(x=bucket, y=count, fill=treatment)) + geom_bar(stat="identity", position="identity", alpha=.5)  +
  scale_y_log10() +
  theme_bw() +
  theme(axis.text = element_text(size=40),
        axis.title = element_text(size=40))
dev.off()
