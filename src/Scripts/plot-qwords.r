library("ggplot2")
setwd("~/dev/BitFunnel/src/Scripts")

png(filename="qwords.png",width=1600,height=1200)
queries <- read.csv(header=TRUE, file="/tmp/QueryPipelineStatistics.csv")
pos = seq(1, length(queries$quadwords))
df <- data.frame(pos, queries$quadwords)

ggplot(df, aes(x=pos,y=queries.quadwords)) +
theme_bw() +
geom_point(alpha=1/100) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40),
      legend.title=element_text(size=40),
      legend.text=element_text(size=40))
dev.off()


png(filename="qwords-solid.png",width=1600,height=1200)
ggplot(df, aes(x=pos,y=queries.quadwords)) +
theme_bw() +
geom_point(alpha=1) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40),
      legend.title=element_text(size=40),
      legend.text=element_text(size=40))
dev.off()