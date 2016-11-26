library("ggplot2")
library("reshape")
setwd("~/dev/BitFunnel/src/Scripts")


queries <- read.csv(header=TRUE, file="/tmp/QueryPipelineStatistics.csv")
# Create column to graph vs. term position.
pos = seq(1, length(queries$quadwords))
df_temp <- data.frame(pos, queries$quadwords, queries$cachelines)

# Rename columns to have shorter names.
names(df_temp)[names(df_temp) == 'queries.quadwords'] <- 'quadwords'
names(df_temp)[names(df_temp) == 'queries.cachelines'] <- 'cachelines'

# Create dataframe in fully normalized form.
df <- melt(df_temp, id=c("pos"))

# Rename meaningless column name.
names(df)[names(df) == 'variable'] <- 'access_type'

png(filename="qwords.png",width=1600,height=1200)
ggplot(df, aes(x=pos,y=value,colour=access_type)) +
theme_bw() +
geom_point(alpha=1/150) +
guides(colour = guide_legend(override.aes = list(alpha = 1))) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40),
      legend.title=element_text(size=40),
      legend.text=element_text(size=40))
dev.off()

df_temp$quadwords <- df_temp$quadwords / 8
df <- melt(df_temp, id=c("pos"))
names(df)[names(df) == 'variable'] <- 'access_type'

png(filename="qwords-divided.png",width=1600,height=1200)
ggplot(df, aes(x=pos,y=value,colour=access_type)) +
theme_bw() +
geom_point(alpha=1/150) +
guides(colour = guide_legend(override.aes = list(alpha = 1))) +
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