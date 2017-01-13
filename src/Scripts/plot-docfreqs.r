library("ggplot2")
setwd("~/dev/BitFunnel/src/Scripts")

df <- read.csv(header=TRUE, file="/tmp/compare-docfreqs.csv")

print("Creating plot.")
png(filename="docfreqs.png",width=1600,height=1200)
ggplot(df, aes(x=idf1,y=idf2)) +
theme_minimal() +
# scale_x_log10() +
# scale_y_log10() +
geom_point(alpha=1/10) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40))
dev.off()

