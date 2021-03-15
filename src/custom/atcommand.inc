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
* Battleground Leader Commands
*------------------------------------------*/
ACMD_FUNC(order)
{
	nullpo_retr(-1,sd);
	memset(atcmd_output, '\0', sizeof(atcmd_output));
	if( !message || !*message )
	{
		clif_displaymessage(fd, "Please, enter a message (usage: @order <message>).");
		return -1;
	}
 
	if( map_getmapflag(sd->bl.m, MF_BATTLEGROUND) )
	{
		if( !sd->bmaster_flag )
		{
			clif_displaymessage(fd, "This command is reserved for Team Leaders Only.");
			return -1;
		}
		if (battle_config.bg_order_behavior)
			sprintf(atcmd_output, "%s: %s", sd->status.name, message);
		else
			sprintf(atcmd_output, "Team Leader: %s", message);
		clif_broadcast2(&sd->bl, atcmd_output, (int)strlen(atcmd_output)+1, sd->bmaster_flag->color, 0x190, 20, 0, 0, BG);
	}
	else
	{
		if( !sd->state.gmaster_flag )
		{
			clif_displaymessage(fd, "This command is reserved for Guild Leaders Only.");
			return -1;
		}
		clif_broadcast2(&sd->bl, message, (int)strlen(message)+1, 0xFF0000, 0x190, 20, 0, 0, GUILD);
	}

	return 0;
}

ACMD_FUNC(leader)
{
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1,sd);
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd,"Please enter a player name (usage: @leader <char name/ID>).");
		return -1;
	}

	if( !sd->bmaster_flag )
		clif_displaymessage(fd, "This command is reserved for Team Leaders Only.");
	else if( sd->ud.skilltimer != INVALID_TIMER )
		clif_displaymessage(fd, "Command not allow while casting a skill.");
	else if( !message || !*message )
		clif_displaymessage(fd, "Please, enter the new Leader name (usage: @leader <name>).");
	else if((pl_sd=map_nick2sd(atcmd_player_name,true)) == NULL && (pl_sd=map_charid2sd(atoi(atcmd_player_name))) == NULL)
		clif_displaymessage(fd, "Character not found.");
	else if( sd->bg_id != pl_sd->bg_id )
		clif_displaymessage(fd, "Destination Player is not in your Team.");
	else if( sd == pl_sd )
		clif_displaymessage(fd, "You are already the Team Leader.");
	else
	{ // Everytest OK!
		sprintf(atcmd_output, "Team Leader transfered to [%s]", pl_sd->status.name);
		clif_broadcast2(&sd->bl, atcmd_output, (int)strlen(atcmd_output)+1, sd->bmaster_flag->color, 0x190, 20, 0, 0, BG);

		sd->bmaster_flag->leader_char_id = pl_sd->status.char_id;
		pl_sd->bmaster_flag = sd->bmaster_flag;
		sd->bmaster_flag = NULL;

		clif_name_area(&sd->bl);
		clif_name_area(&pl_sd->bl);
		return 0;
	}
	return -1;
}

ACMD_FUNC(reportafk)
{
	struct map_session_data *pl_sd = NULL;
	nullpo_retr(-1,sd);
	memset(atcmd_player_name, '\0', sizeof(atcmd_player_name));
	
	if (!message || !*message || sscanf(message, "%23[^\n]", atcmd_player_name) < 1) {
		clif_displaymessage(fd,"Please enter a player name (usage: @reportafk <char name/ID>).");
		return -1;
	}
	if( !sd->bg_id )
		clif_displaymessage(fd, "This command is reserved for Battleground Only.");
	else if( !sd->bmaster_flag && battle_config.bg_reportafk_leaderonly )
		clif_displaymessage(fd, "This command is reserved for Team Leaders Only.");
	else if( !message || !*message )
		clif_displaymessage(fd, "Please, enter the character name (usage: @reportafk <name>).");
	else if((pl_sd=map_nick2sd(atcmd_player_name,true)) == NULL && (pl_sd=map_charid2sd(atoi(atcmd_player_name))) == NULL)
		clif_displaymessage(fd, "Character not found");
	else if( sd->bg_id != pl_sd->bg_id )
		clif_displaymessage(fd, "Destination Player is not in your Team.");
	else if( sd == pl_sd )
		clif_displaymessage(fd, "You cannot kick yourself.");
	else if( pl_sd->state.bg_afk == 0 )
		clif_displaymessage(fd, "The player is not AFK on this Battleground.");
	else
	{ // Everytest OK!
		struct battleground_data *bgd;
		if( (bgd = bg_team_search(sd->bg_id)) == NULL )
			return -1;

		bg_team_leave(pl_sd,2);
		clif_displaymessage(pl_sd->fd, "You have been kicked from Battleground because of your AFK status.");
		pc_setpos(pl_sd,pl_sd->status.save_point.map,pl_sd->status.save_point.x,pl_sd->status.save_point.y,CLR_TELEPORT);

		sprintf(atcmd_output, "- AFK [%s] Kicked by @reportafk command-", pl_sd->status.name);
		clif_broadcast2(&sd->bl, atcmd_output, (int)strlen(atcmd_output)+1, bgd->color, 0x190, 20, 0, 0, BG);
		return 0;
	}
	return -1;
}

ACMD_FUNC(listenbg)
{
	if (sd->state.bg_listen)
	{
		sd->state.bg_listen = 0;
		clif_displaymessage(fd, "You will receive Battleground announcements");
	}
	else
	{
		sd->state.bg_listen = 1;
		clif_displaymessage(fd, "You will not receive Battleground announcements.");
	}

	return 0;
}

/*==========================================
* Guild Skill Usage for Guild Masters
*------------------------------------------*/
ACMD_FUNC(guildskill)
{
	int i, skillnum = 0, skilllv = 1;
	t_tick tick = gettick();
	struct guild *g;
	char output[128];
	
	const struct { char skillstr[3]; int id; } skills[] = {
		{ "BO",	10010 },
		{ "RG",	10011 },
		{ "RS",	10012 },
		{ "EC",	10013 },
	};
	
	for( i = 0; i < ARRAYLENGTH(skills); i++ )
	{
		if( strncmpi(message, skills[i].skillstr, 3) == 0 )
		{
			skillnum = skills[i].id;
			break;
		}
	}
	if( !skillnum )
	{
		clif_displaymessage(fd, "Invalid Skill string. Use @guildskill EC/RS/RG/BO");
		return -1;
	}
	
	if( map_getmapflag(sd->bl.m, MF_GVG_CASTLE) ||map_getmapflag(sd->bl.m, MF_BATTLEGROUND) ){
		if( sd->state.gmaster_flag && (g = sd->guild) != NULL ){
			if(guild_checkskill(g, skillnum) < 1){
				skilllv = 0;
				sprintf(output, "The guild does not have the skill %s.", skill_get_desc(skillnum));
				clif_displaymessage(sd->fd, output);
			}
		} else {
			clif_displaymessage(fd, "This command is reserved for Guild Leaders Only.");
			return -1;
		}	
	}
	else
	{
		clif_displaymessage(fd, "This command is only available for Castles.");
		return -1;		
	}
	
	if( pc_cant_act(sd) || pc_issit(sd) || skill_isNotOk(skillnum, sd) || sd->ud.skilltimer != -1 || sd->sc.option&(OPTION_WEDDING|OPTION_XMAS|OPTION_SUMMER) || sd->sc.data[SC_BASILICA] )
		return -1;
	
	if( DIFF_TICK(tick, sd->ud.canact_tick) < 0 )
		return -1;

	if( sd->menuskill_id )
	{
		if( sd->menuskill_id == SA_TAMINGMONSTER )
			sd->menuskill_id = sd->menuskill_val = 0; //Cancel pet capture.
		else if( sd->menuskill_id != SA_AUTOSPELL )
			return -1; //Can't use skills while a menu is open.
	}
	
	sd->skillitem = sd->skillitemlv = 0;
	if( skilllv ) unit_skilluse_id(&sd->bl, sd->bl.id, skillnum, skilllv);
	return 0;
}

ACMD_FUNC(bgrank)
{
	int i;

	clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], "============ BATTLEGROUND RANK ============", false, SELF);
	for( i = 0; i < MAX_FAME_LIST; i++ )
	{
		if (!bg_fame_list[i].id)
			sprintf(atcmd_output,"[ %d ] None        :    0 POINT",i+1);
		else
			sprintf(atcmd_output,"[ %d ] %s        :    %d POINT",i+1,bg_fame_list[i].name,bg_fame_list[i].fame);
		clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], atcmd_output, false, SELF);
	}
	sprintf(atcmd_output,"MY POINT    :    %d POINT",sd->status.bgstats.points);
	clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], "===========================================", false, SELF);
	clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], atcmd_output, false, SELF);
	clif_messagecolor(&sd->bl, color_table[COLOR_LIGHT_GREEN], "===========================================", false, SELF);
	return 0;
}

ACMD_FUNC(woerank)
{
	int i;

	clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], "============ WAR OF EMPERIUM RANK ============", false, SELF);
	for( i = 0; i < MAX_FAME_LIST; i++ )
	{
		if (!woe_fame_list[i].id)
			sprintf(atcmd_output,"[ %d ] None        :    0 POINT",i+1);
		else
			sprintf(atcmd_output,"[ %d ] %s        :    %d POINT",i+1,woe_fame_list[i].name,woe_fame_list[i].fame);
		clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], atcmd_output, false, SELF);
	}
	sprintf(atcmd_output,"MY POINT    :    %d POINT",sd->status.wstats.points);
	clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], "===========================================", false, SELF);
	clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], atcmd_output, false, SELF);
	clif_messagecolor(&sd->bl, color_table[COLOR_CYAN], "===========================================", false, SELF);
	return 0;
}

ACMD_FUNC(bgstats)
{
	pc_battle_stats(sd,sd,1);
	return 0;
}

ACMD_FUNC(woestats)
{
	pc_battle_stats(sd,sd,2);
	return 0;
}
