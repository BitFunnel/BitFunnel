library("ggplot2")
library("reshape2")
setwd("~/dev/BitFunnel/src/Scripts")

# See
# https://www.r-bloggers.com/choosing-colour-palettes-part-ii-educated-choices/
# for color information.

queries <- read.csv(header=TRUE, file="/tmp/Memory.csv")

# queries <- read.csv(header=TRUE, file="/tmp/QueryPipelineStatistics.csv")
# # Create column to graph vs. term position.
# pos = seq(1, length(queries$quadwords))
# df_temp <- data.frame(pos, queries$quadwords, queries$cachelines)

# # Rename columns to have shorter names.
# names(df_temp)[names(df_temp) == 'queries.quadwords'] <- 'quadwords'
# names(df_temp)[names(df_temp) == 'queries.cachelines'] <- 'cachelines'

# Create dataframe in fully normalized form.
df <- melt(queries, measure.vars=c("Quadwords","Cachelines","ExpectedCachelines"), id.vars="TermPos")

# Rename meaningless column name.
names(df)[names(df) == 'variable'] <- 'AccessType'

png(filename="qwords.png",width=1600,height=1200)
ggplot(df, aes(x=TermPos,y=value,colour=AccessType)) +
scale_fill_brewer(palette="Set1") + # doesn't work :-(
theme_minimal() +
geom_point(alpha=1/150) +
guides(colour = guide_legend(override.aes = list(alpha = 1))) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40),
      legend.title=element_text(size=40),
      legend.text=element_text(size=40))
dev.off()

queries$Quadwords <- queries$Quadwords / 8
# df <- melt(queries, measure.vars=c("Quadwords","Cachelines","ExpectedCachelines"), id.vars="TermPos")
df <- melt(queries, measure.vars=c("Quadwords","ExpectedCachelines"), id.vars="TermPos")
names(df)[names(df) == 'variable'] <- 'AccessType'

png(filename="qwords-divided.png",width=1600,height=1200)
ggplot(df, aes(x=TermPos,y=value,colour=AccessType)) +
scale_fill_brewer("Set1") + # doesn't work :-(
theme_minimal() +
geom_point(alpha=1/150) +
guides(colour = guide_legend(override.aes = list(alpha = 1))) +
theme(axis.text = element_text(size=40),
      axis.title = element_text(size=40),
      legend.title=element_text(size=40),
      legend.text=element_text(size=40))
dev.off()


# png(filename="qwords-solid.png",width=1600,height=1200)
# ggplot(df, aes(x=TermPos,y=value)) +
# theme_bw() +
# geom_point(alpha=1) +
# theme(axis.text = element_text(size=40),
#       axis.title = element_text(size=40),
#       legend.title=element_text(size=40),
#       legend.text=element_text(size=40))
# dev.off()