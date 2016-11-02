library("ggplot2")
setwd("~/dev/BitFunnel/src/Scripts")
df <- read.csv(header=TRUE, file="history.csv")

png(filename="bitfunnel-progress.png",width=1600,height=1200)
df$date <- as.Date(df$date)
ggplot(df, aes(date, wc)) + geom_line(size=3) + scale_x_date() +
    labs(x = "Date", y = "Lines of code") +
    stat_smooth(se=FALSE) +
    theme_bw() +
    theme(axis.text = element_text(size=40),
          axis.title = element_text(size=40))
dev.off()

endDate <- as.Date("2017/07/01")
png(filename="bitfunnel-extrapolation.png",width=1600,height=1200)
ggplot(df, aes(date, wc)) + geom_line(size=3) +
    scale_x_date(limits=c(df$date[1],endDate)) +
    labs(x = "Date", y = "Lines of code") +
    stat_smooth(method="lm",fullrange=TRUE,se=FALSE) +
    geom_hline(yintercept=72000,color="green") +
    geom_hline(yintercept=144000,color="red") +
    theme_bw() +
    theme(axis.text = element_text(size=40),
          axis.title = element_text(size=40))
dev.off()