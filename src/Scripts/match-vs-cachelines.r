library("ggplot2")
library("reshape2")
setwd("~/dev/BitFunnel/src/Scripts")

interpreter <- read.csv(header=TRUE, file="/tmp/int/QueryPipelineStatistics.csv")
compiler <- read.csv(header=TRUE, file="/tmp/comp/QueryPipelineStatistics.csv")
df <- data.frame(interpreter$cachelines, compiler$match)
names(df)[names(df) == 'interpreter.cachelines'] <- 'Cachelines'
names(df)[names(df) == 'compiler.match'] <- 'MatchTime'

png(filename="match-vs-cachelines.png",width=1600,height=1200)
ggplot(df, aes(x=Cachelines,y=MatchTime)) +
theme_minimal() +
geom_point(alpha=1/50) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40)) +
ylim(0, 0.00004)
dev.off()
