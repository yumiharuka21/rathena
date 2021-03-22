// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

/**
 * Custom Script Commands
 * Place the body of custom script commands in this file.
 * Format:
 *	BUILDIN_FUNC(command_name)
 *	{
 *		<code>;
 *	}
 **/

//BUILDIN_FUNC(example)
//{
//	ShowInfo("buildin_example: It works!\n");
//	script_pushint(st,1);
//	return 0;
//}

/*==========================================
 * @homload [Haruka Mayumi]
 *------------------------------------------*/
BUILDIN_FUNC(homload)
{
	int hom_id;
	TBL_PC *sd;
	
	if(!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	
	if(!pc_checkskill(sd,238)){
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	
	hom_id = script_getnum(st,2);
	
	if(sd->hd != NULL){
		hom_save(sd->hd);
		hom_delete(sd->hd, -1);
	}

	memset(&sd->hd,0,sizeof(struct homun_data));
	sd->status.hom_id = hom_id;
	hom_call(sd);
	
	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}

/*==========================================
 * @homsave [Haruka Mayumi]
 *------------------------------------------*/
BUILDIN_FUNC(homsave)
{
	TBL_PC *sd;

	if(!script_rid2sd(sd))
		return SCRIPT_CMD_SUCCESS;
	
	if(!pc_checkskill(sd,238)){
		script_pushint(st, 0);
		return SCRIPT_CMD_SUCCESS;
	}
	
	if(sd->hd != NULL){
		hom_save(sd->hd);
		hom_delete(sd->hd, -1);
	}

	memset(&sd->hd,0,sizeof(struct homun_data));

	script_pushint(st, 1);
	return SCRIPT_CMD_SUCCESS;
}