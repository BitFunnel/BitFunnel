library("ggplot2")
setwd("~/dev/BitFunnel/build-ninja/") # TODO: make path general.
df <- read.csv(header=TRUE, file="sum.csv")

png(filename="true-matches.png",width=1600,height=1200)
ggplot(df, aes(TermPos,TruePositives)) +
    geom_point(alpha=1/5) +
    theme_bw() +
    theme(axis.text = element_text(size=40),
          axis.title = element_text(size=40))
dev.off()


png(filename="false-rate.png",width=1600,height=1200)
ggplot(df, aes(TermPos,FalseRate)) +
    geom_point(alpha=1/100) +
    theme_bw() +
    theme(axis.text = element_text(size=40),
          axis.title = element_text(size=40))
dev.off()


png(filename="false-matches.png",width=1600,height=1200)
ggplot(df, aes(TermPos,FalsePositives)) +
    geom_point(alpha=1/5) +
    theme_bw() +
    theme(axis.text = element_text(size=40),
          axis.title = element_text(size=40))
dev.off()
