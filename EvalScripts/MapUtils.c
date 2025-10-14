get_nearest_sector(current_sector) {
    num_walls = mapgetsect(current_sector, MAP_N);
    
    for(w = 0; w < num_walls; w++) {
        opposing_sector = mapgetwall(current_sector, w, MAP_OPPS);
        if(opposing_sector >= 0) {
            return opposing_sector;
        }
    }
    
    return -1; // No adjacent sector found
}