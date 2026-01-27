library("irace")

source("tuning/repair.R")

scenario <- readScenario(filename = "tuning/scenario.txt", scenario = list())

scenario$repairConfiguration <- "repair_neighborhoods"

args <- commandArgs(trailingOnly = TRUE)
if (length(args) > 0) {
  scenario <- readScenario(filename = "tuning/scenario.txt", scenario = scenario)
}

irace(scenario = scenario)
