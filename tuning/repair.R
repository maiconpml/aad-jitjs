repair_neighborhoods <- function(configuration, parameters, digits) {
  all_nhoods <- c("SWAP_ADJ", "SWAP_RAND", "INSERT_RAND", 
                  "SWAP_PENAL", "INSERT_PENAL", 
                  "CRITICAL_OPER", "CRITICAL_OPER_ALT")
  
  # Identify the neighborhood columns
  nhood_cols <- paste0("nhood", 1:7)
  
  valid_cols <- intersect(nhood_cols, colnames(configuration))
  
  if (length(valid_cols) == 0) return(configuration)

  # Iterate over each configuration in the batch
  for (i in 1:nrow(configuration)) {
    # Extract current values for this row
    current_vals <- as.character(configuration[i, valid_cols])
    
    # Identify active parameters
    active_mask <- !is.na(current_vals)
    
    if (sum(active_mask) > 1) {
      active_vals <- current_vals[active_mask]
      
      # Check for duplicates among active values
      if (any(duplicated(active_vals))) {
        # Values currently used
        used_vals <- unique(active_vals)
        # Values available to swap in
        available_vals <- setdiff(all_nhoods, used_vals)
        
        is_dup <- duplicated(active_vals)
        
        avail_idx <- 1
        
        # Replace duplicates
        for (k in seq_along(active_vals)) {
          if (is_dup[k]) {
            if (avail_idx <= length(available_vals)) {
              active_vals[k] <- available_vals[avail_idx]
              avail_idx <- avail_idx + 1
            }
          }
        }
        
        # Update the configuration row
        configuration[i, valid_cols[active_mask]] <- active_vals
      }
    }
  }
  
  return(configuration)
}
