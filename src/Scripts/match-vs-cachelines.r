library("broom")
library("ggplot2")
library("reshape2")
setwd("~/dev/BitFunnel/src/Scripts")

args = commandArgs(trailingOnly=TRUE)
if (length(args) != 4) {
   stop("Required args: [interpreter QueryPipelineStats filename], [compiler QPS filename], [cachelines vs time filename], [matches vs. time filename]", call.=FALSE)
}
int_name = args[1]
comp_name = args[2]
out_name1 = args[3]
out_name2 = args[4]

print("Reading input.")
interpreter <- read.csv(header=TRUE, file=int_name)
compiler <- read.csv(header=TRUE, file=comp_name)

df <- data.frame(interpreter$cachelines, compiler$match)
names(df)[names(df) == 'interpreter.cachelines'] <- 'Cachelines'
names(df)[names(df) == 'compiler.match'] <- 'MatchTime'

# print("Plotting cachelines vs. time.")
# png(filename=out_name1,width=1600,height=1200)
# ggplot(df, aes(x=Cachelines,y=MatchTime)) +
# theme_minimal() +
# geom_point(alpha=1/100) +
# theme(axis.text = element_text(size=40),
#       axis.title = element_text(size=40)) +
# ylim(0, 0.001)
# dev.off()

# print("Plotting matches vs. time.")
# png(filename=out_name2,width=1600,height=1200)
# ggplot(compiler, aes(x=matches,y=match)) +
# theme_minimal() +
# geom_point(alpha=1/20) +
# theme(axis.text = element_text(size=40),
#       axis.title = element_text(size=40))
# dev.off()

print("Computing cacheline regression.")
df <- data.frame(interpreter$cachelines, compiler$matches, compiler$match)
names(df)[names(df) == 'interpreter.cachelines'] <- 'Cachelines'
names(df)[names(df) == 'compiler.matches'] <- 'Matches'
names(df)[names(df) == 'compiler.match'] <- 'Time'

fit <- lm(Time ~ Matches, data=df)
print(summary(fit))

fit <- lm(Time ~ Cachelines, data=df)
print(summary(fit))

fit <- lm(Time ~ ., data=df)
print(summary(fit))

print("Residual plot.")
df <- augment(fit)

# # TODO: don't hardcode filename.
# png(filename="time-residual.png",width=1600,height=1200)
# ggplot(df, aes(x = .fitted, y = .resid)) +
# theme_minimal() +
# geom_point(alpha=1/10) +
# theme(axis.text = element_text(size=40),
#       axis.title = element_text(size=40))
# dev.off()      

print("Computing quadword regression.")
df <- data.frame(interpreter$quadwords, compiler$matches, compiler$match)
names(df)[names(df) == 'interpreter.quadwords'] <- 'Quadwords'
names(df)[names(df) == 'compiler.matches'] <- 'Matches'
names(df)[names(df) == 'compiler.match'] <- 'Time'

fit <- lm(Time ~ Matches, data=df)
print(summary(fit))

fit <- lm(Time ~ Quadwords, data=df)
print(summary(fit))

fit <- lm(Time ~ ., data=df)
print(summary(fit))


df <- data.frame(interpreter$cachelines, compiler$match)
names(df)[names(df) == 'interpreter.quadwords'] <- 'Quadwords'
names(df)[names(df) == 'compiler.match'] <- 'MatchTime'

