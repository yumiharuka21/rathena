// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Atcommands
 * Place the body of custom atcommands in this file.
 * Format:
 *	ACMD_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//ACMD_FUNC(newcommand)
//{
//	clif_displaymessage(fd, "It works!");
//	clif_specialeffect(&sd->bl, EF_HEARTCASTING, AREA);
//	return 0;
//}

/*==========================================
 * Created by : Haruka Mayumi
 *------------------------------------------*/
ACMD_FUNC(whosell)
{
	struct map_session_data* pl_sd;
	struct s_mapiterator* iter;
	struct item_data *item_data;
	struct s_item_link itemldata;
	char item_name[100], vend_type[100];
	t_itemid item_id = 0;
	int type = 0, refine = 0, card = 0, num = 0, count = 0;
	bool flag = 1; // place dot on the minimap?
	
	nullpo_retr(-1, sd);

	memset(item_name, '\0', sizeof(item_name));
		
	if (!message || !*message || (
		sscanf(message, "%11d \"%99[^\"]\" %11d %11d",(battle_config.extended_vending ? &type:0), item_name, &refine, &card) < 4 &&
		sscanf(message, "%11d %99s %11d %11d",(battle_config.extended_vending ? &type:0), item_name, &refine, &card) < 4 ))
	{
		clif_displaymessage(fd, "Invalid usage of @whosell command");
		if(battle_config.extended_vending){
			clif_displaymessage(fd, "Usage: @whosell <Type ID> <item name/item id> <refine> <card>");
			clif_displaymessage(fd, "Types Available: Type ID -> Name"); // Invalid bound type
			if(battle_config.item_zeny){
				sprintf(atcmd_output,"-> [%d] Zeny", battle_config.item_zeny);
				clif_displaymessage(fd, atcmd_output);
			}
			if(battle_config.item_cash){
				sprintf(atcmd_output,"-> [%d] Cash Points", battle_config.item_cash);
				clif_displaymessage(fd, atcmd_output);
			}
			for (const auto &vend : item_vend){
				if(vend.itemid){
					sprintf(vend_type,"-> [%d] %s", vend.itemid, itemdb_ename(vend.itemid));
					clif_displaymessage(fd, vend_type);
				}
			}
		} else
			clif_displaymessage(fd, "Usage: @whosell <item name/item id> <refine> <card>");
		return -1;
	}

	if ((item_data = itemdb_searchname(item_name)) != NULL ||
	    (item_data = itemdb_exists(strtoul(item_name, nullptr, 10))) != NULL)
		item_id = item_data->nameid;

	if (refine < 0 || refine > 10){
		clif_displaymessage(fd, "Refine out of bounds: 0 - 10");
		return -1;
	}
		
	if (card){
		if((item_data = itemdb_exists(card)) == NULL || item_data->type != IT_CARD){
			clif_displaymessage(fd, "Invalid Card ID");
			return -1;
		}
	}
		
	if (item_id > 500) {
		//script_cleararray_pc( sd, "@vendor_cid" );
		iter = mapit_getallusers();
		for (pl_sd = (TBL_PC*)mapit_first(iter); mapit_exists(iter); pl_sd = (TBL_PC*)mapit_next(iter)){
			if (pl_sd->vender_id) {	 // check if player is vending
				if(battle_config.extended_vending && pl_sd->vend_loot != type)
					continue;
				for (int j = 0; j < pl_sd->vend_num; j++) {
					if ((item_data = itemdb_exists(pl_sd->cart.u.items_cart[pl_sd->vending[j].index].nameid)) == NULL)
						continue;
					if(pl_sd->cart.u.items_cart[pl_sd->vending[j].index].nameid != item_id)
						continue;
					if(card)
						if((item_data->type != IT_ARMOR && item_data->type != IT_WEAPON) ||
							(pl_sd->cart.u.items_cart[pl_sd->vending[j].index].card[0] != card &&
							pl_sd->cart.u.items_cart[pl_sd->vending[j].index].card[1] != card &&
							pl_sd->cart.u.items_cart[pl_sd->vending[j].index].card[2] != card &&
							pl_sd->cart.u.items_cart[pl_sd->vending[j].index].card[3] != card))
							continue;
					if(refine)
						if((item_data->type != IT_ARMOR && item_data->type != IT_WEAPON) || pl_sd->cart.u.items_cart[pl_sd->vending[j].index].refine != refine)
							continue;
					count++;
					flag = 1;
					memset(&itemldata, 0, sizeof(itemldata));
					for(int k = 0; k < 4; k++)
						itemldata.cards[k] = pl_sd->cart.u.items_cart[pl_sd->vending[j].index].card[k];
					if (itemldata.cards[0] || itemldata.cards[1] || itemldata.cards[2] || itemldata.cards[3])
						itemldata.flag.cards = 1;
#if PACKETVER >= 20150225
					for (int l = 0; l < MAX_ITEM_RDM_OPT; l++)
						if(pl_sd->cart.u.items_cart[pl_sd->vending[j].index].option[l].id)
							itemldata.flag.options = 1;
					memcpy(&itemldata.options, &pl_sd->cart.u.items_cart[pl_sd->vending[j].index].option, sizeof(pl_sd->cart.u.items_cart[pl_sd->vending[j].index].option));
#endif
					std::string itemlstr = createItemLink(item_id, pl_sd->cart.u.items_cart[pl_sd->vending[j].index].refine, &itemldata);
					char *str = (char *)aMalloc((itemlstr.size() + 1) * sizeof(char));
					safestrncpy(str, itemlstr.c_str(), itemlstr.size() + 1);
					snprintf(atcmd_output, CHAT_SIZE_MAX,"[Seller: %s] %s at %s %d %d for %u %s",pl_sd->status.name
							,str
							,mapindex_id2name(pl_sd->mapindex)
							,pl_sd->bl.x,pl_sd->bl.y
							,pl_sd->vending[j].value
							,battle_config.extended_vending ? itemdb_ename(type):"Zeny");
					clif_displaymessage(fd, atcmd_output);
					//pc_setreg(sd,reference_uid(add_str("@vendor_cid"), count), pl_sd->status.char_id);
				}
				if (flag && pl_sd->mapindex == sd->mapindex) {
					clif_viewpoint(sd, 1, 1, pl_sd->bl.x, pl_sd->bl.y, ++num, 0xFFFFFF);
					flag = 0;
				}
			}
		}
		mapit_free(iter);
		if(count > 0) {
			if(battle_config.extended_vending)
				snprintf(atcmd_output, CHAT_SIZE_MAX, "============= Found %d entries [%s] =============", count, itemdb_ename(type));
			else
				snprintf(atcmd_output, CHAT_SIZE_MAX, "===================== Found %d entries. =====================", count);
			clif_displaymessage(fd, atcmd_output);
			clif_displaymessage(fd, "You can use @jumptosell <seller name> to warp at vendor location");
		} else
			clif_displaymessage(fd, "Nobody is selling it now.");
		return 0;
	} else {
		clif_displaymessage(fd, msg_txt(sd,19)); // Invalid item ID or name.
		return -1;
	}

	return 0;
}

/*==========================================
 * Created by : Haruka Mayumi
 *------------------------------------------*/
ACMD_FUNC(jumptosell)
{
	nullpo_retr(-1, sd);

	struct map_session_data *pl_sd = NULL;
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd, "Please enter a vendor name (usage: @jumptosell <vendor name>)");
		return -1;
	}
	
	if((pl_sd=map_nick2sd(atcmd_player_name,true)) == NULL)
	{
		clif_displaymessage(fd, msg_txt(sd,3)); // Character not found.
		return -1;
	}
	
	if (!pl_sd->vender_id)
	{
		sprintf(atcmd_output,"%s is not vending anymore",pl_sd->status.name);
		clif_displaymessage(fd, atcmd_output);
		return -1;
	}
	
	if (pl_sd->bl.m >= 0 && map_getmapflag(pl_sd->bl.m, MF_NOWARPTO) && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE))
	{
		clif_displaymessage(fd, msg_txt(sd,247));	// You are not authorized to warp to this map.
		return -1;
	}

	if (sd->bl.m >= 0 && map_getmapflag(sd->bl.m, MF_NOWARP) && !pc_has_permission(sd, PC_PERM_WARP_ANYWHERE))
	{
		clif_displaymessage(fd, msg_txt(sd,248));	// You are not authorized to warp from your current map.
		return -1;
	}

	if( pc_isdead(sd) )
	{
		clif_displaymessage(fd, msg_txt(sd,664)); // You cannot use this command when dead.
		return -1;
	}

	pc_setpos(sd, pl_sd->mapindex, pl_sd->bl.x, pl_sd->bl.y, CLR_TELEPORT);
	sprintf(atcmd_output, msg_txt(sd,4), pl_sd->status.name); // Jumped to %s
 	clif_displaymessage(fd, atcmd_output);

	return 0;
}