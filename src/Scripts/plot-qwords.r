library("ggplot2")
library("reshape")
setwd("~/dev/BitFunnel/src/Scripts")


queries <- read.csv(header=TRUE, file="/tmp/QueryPipelineStatistics.csv")
pos = seq(1, length(queries$quadwords))
df_temp <- data.frame(pos, queries$quadwords, queries$cachelines)
df <- melt(df_temp, id=c("pos"))

png(filename="qwords.png",width=1600,height=1200)
ggplot(df, aes(x=pos,y=value,colour=factor(variable))) +
theme_bw() +
geom_point(alpha=1/150) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40),
      legend.title=element_text(size=40),
      legend.text=element_text(size=40))
dev.off()

df_temp$queries.quadwords <- df_temp$queries.quadwords / 8
df <- melt(df_temp, id=c("pos"))

png(filename="qwords-divided.png",width=1600,height=1200)
ggplot(df, aes(x=pos,y=value,colour=factor(variable))) +
theme_bw() +
geom_point(alpha=1/150) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40),
      legend.title=element_text(size=40),
      legend.text=element_text(size=40))
dev.off()


# png(filename="qwords-solid.png",width=1600,height=1200)
# ggplot(df, aes(x=pos,y=value)) +
# theme_bw() +
# geom_point(alpha=1) +
# theme(axis.text = element_text(size=40),
#       axis.title = element_text(size=40),
#       legend.title=element_text(size=40),
#       legend.text=element_text(size=40))
# dev.off()