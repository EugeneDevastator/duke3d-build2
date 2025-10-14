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

get_first_sprite_with_tag(sector_index, tag) {
    sprite_index;
    
    // Iterate through all sprites in the sector
    for (sprite_index = mapgetsect(sector_index, MAP_HEADSPRI); 
         sprite_index >= 0; 
         sprite_index = mapgetspri(sprite_index, MAP_N)) {
        
        // Check if sprite has lo-tag 11 
        if (mapgetspri(sprite_index, MAP_TAG) % 65536 == 11) {
           debugnum2=sprite_index;
            return sprite_index;
        }
    }
    
    return -1; // No sprite with tag 11 found
}
