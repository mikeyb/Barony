/*-------------------------------------------------------------------------------

	BARONY
	File: clickdescription.cpp
	Desc: contains clickDescription()

	Copyright 2013-2016 (c) Turning Wheel LLC, all rights reserved.
	See LICENSE for details.

-------------------------------------------------------------------------------*/

#include "../main.hpp"
#include "../game.hpp"
#include "../stat.hpp"
#include "../items.hpp"
#include "../magic/magic.hpp"
#include "../monster.hpp"
#include "../net.hpp"
#include "interface.hpp"

/*-------------------------------------------------------------------------------

	clickDescription
	
	reports the name of the entity clicked on in use mode

-------------------------------------------------------------------------------*/

void clickDescription(int player, Entity *entity) {
	Stat *stat;
	Item *item;
	Uint32 uidnum;
	
	if( entity==NULL ) {
		if( !(*inputPressed(impulses[IN_ATTACK])) || shootmode )
			return;
		if( omousex<camera.winx || omousex>=camera.winx+camera.winw || omousey<camera.winy || omousey>=camera.winy+camera.winh )
			return;
		if (openedChest[clientnum])
			if (omousex > CHEST_INVENTORY_X && omousex < CHEST_INVENTORY_X + inventoryChest_bmp->w && omousey > CHEST_INVENTORY_Y && omousey < CHEST_INVENTORY_Y + inventoryChest_bmp->h)
				return; //Click falls inside the chest inventory GUI.
		if (identifygui_active)
			if (omousex > IDENTIFY_GUI_X && omousex < IDENTIFY_GUI_X + identifyGUI_img->w && omousey > IDENTIFY_GUI_Y && omousey < IDENTIFY_GUI_Y + identifyGUI_img->h)
				return; //Click falls inside the identify item gui.
		if (book_open)
			if (mouseInBounds(BOOK_GUI_X, BOOK_GUI_X + bookgui_img->w, BOOK_GUI_Y, BOOK_GUI_Y + bookgui_img->h))
				return; //Click falls inside the book GUI.
		if (gui_mode == GUI_MODE_INVENTORY || gui_mode == GUI_MODE_SHOP) {
			if( gui_mode == GUI_MODE_INVENTORY )
				if (mouseInBounds(RIGHTSIDEBAR_X, RIGHTSIDEBAR_X + rightsidebar_titlebar_img->w, RIGHTSIDEBAR_Y, RIGHTSIDEBAR_Y + rightsidebar_height))
					return; //Click falls inside the right sidebar.
			//int x = std::max(character_bmp->w, xres/2-inventory_bmp->w/2);
			//if (mouseInBounds(x,x+inventory_bmp->w,0,inventory_bmp->h))
				//return NULL;
			if( mouseInBounds(INVENTORY_STARTX,INVENTORY_STARTX+INVENTORY_SIZEX*INVENTORY_SLOTSIZE,INVENTORY_STARTY,INVENTORY_STARTY+INVENTORY_SIZEY*INVENTORY_SLOTSIZE) ) {
				// clicked in inventory
				return;
			}
			if( gui_mode == GUI_MODE_SHOP ) {
				int x1 = xres/2-SHOPWINDOW_SIZEX/2, x2 = xres/2+SHOPWINDOW_SIZEX/2;
				int y1 = yres/2-SHOPWINDOW_SIZEY/2, y2 = yres/2+SHOPWINDOW_SIZEY/2;
				if (mouseInBounds(x1,x2,y1,y2))
					return;
			}
		} else if (gui_mode == GUI_MODE_MAGIC) {
			if (magic_GUI_state == 0) {
				//Right, now calculate the spell list's height (the same way it calculates it for itself).
				int height = spell_list_titlebar_bmp->h;
				int numspells = 0;
				node_t *node;
				for (node = spellList.first; node != NULL; node = node->next) {
					numspells++;
				}
				int maxSpellsOnscreen = camera.winh / spell_list_gui_slot_bmp->h;
				numspells = std::min(numspells, maxSpellsOnscreen);
				height += numspells * spell_list_gui_slot_bmp->h;
				int spelllist_y = camera.winy + ((camera.winh / 2) - (height / 2)) + magicspell_list_offset_x;

				if (mouseInBounds(MAGICSPELL_LIST_X, MAGICSPELL_LIST_X + spell_list_titlebar_bmp->w, spelllist_y, spelllist_y + height))
					return;
			}
		}
		if (mouseInBounds(0,224,0,420)) // character sheet
			return;
		int x = xres/2-(status_bmp->w/2);
		if (mouseInBounds(x,x+status_bmp->w,yres-status_bmp->h-hotbar_img->h,yres))
			return;
		*inputPressed(impulses[IN_ATTACK])=0;
		
		if( softwaremode ) {
			entity = clickmap[omousey+omousex*yres];
		} else {
			GLubyte pixel[4];
			glReadPixels(omousex,yres-omousey,1,1,GL_RGBA,GL_UNSIGNED_BYTE,(void *)pixel);
			uidnum = pixel[0] + (((Uint32)pixel[1])<<8) + (((Uint32)pixel[2])<<16) + (((Uint32)pixel[3])<<24);
			entity = uidToEntity(uidnum);
		}
	}
	
	if( entity != NULL ) {
		if( multiplayer != CLIENT ) {
			if( (stat=entity->getStats())==NULL ) {
				Entity *parent = uidToEntity(entity->parent);
				if( entity->behavior==&actPlayerLimb || entity->skill[2]==entity->parent ) {
					if( parent ) {
						if( parent->behavior==&actPlayer || parent->behavior==&actMonster ) {
							Stat *stats = parent->getStats();
							if( stats ) {
								if( strcmp(stats->name,"") ) {
									messagePlayer(player,language[253],language[90+stats->type],stats->name);
								} else {
									messagePlayer(player,language[254],language[90+stats->type]);
								}
							}
						}
					}
				}
				else if( entity->behavior==&actTorch ) {
					messagePlayer(player,language[255]);
				}
				else if( entity->behavior==&actDoor ) {
					messagePlayer(player,language[256]);
				}
				else if( entity->behavior==&actItem ) {
					item = newItem(static_cast<ItemType>(entity->skill[10]),static_cast<Status>(entity->skill[11]),entity->skill[12],entity->skill[13],entity->skill[14],FALSE,NULL);
					if (item) {
						messagePlayer(player,language[257],item->description());
						free(item);
					}
				}
				else if( entity->behavior==&actGoldBag ) {
					if( entity->skill[0]==1 )
						messagePlayer(player,language[258]);
					else
						messagePlayer(player,language[259],entity->skill[0]);
				}
				else if( entity->behavior==&actCampfire) {
					messagePlayer(player,language[260]);
				}
				else if( entity->behavior==&actFountain) {
					messagePlayer(player,language[262]);
				}
				else if( entity->behavior==&actSink) {
					messagePlayer(player,language[263]);
				}
				else if( entity->behavior==&actLadder) {
					messagePlayer(player,language[264]);
				}
				else if( entity->behavior==&actLadderUp) {
					messagePlayer(player,language[265]);
				}
				else if( entity->behavior==&actChest || entity->behavior==&actChestLid ) {
					messagePlayer(player,language[266]);
				}
				else if( entity->behavior==&actGate) {
					messagePlayer(player,language[267]);
				}
				else if( entity->behavior==&actSpearTrap) {
					messagePlayer(player,language[268]);
				}
				else if( entity->behavior==&actSwitch) {
					messagePlayer(player,language[269]);
				}
				else if( entity->behavior==&actBoulder ) {
					messagePlayer(player,language[270]);
				}
				else if( entity->behavior==&actHeadstone ) {
					messagePlayer(player,language[271]);
				}
				else if( entity->behavior==&actPortal || entity->behavior==&actWinningPortal ) {
					messagePlayer(player,language[272]);
				}
				else if( entity->behavior==&actFurniture ) {
					if( entity->skill[0] )
						messagePlayer(player,language[273]);
					else
						messagePlayer(player,language[274]);
				}
			} else {
				if( !strcmp(stat->name,"") ) {
					messagePlayer(player,language[254],language[90+stat->type]);
				} else {
					messagePlayer(player,language[253],language[90+stat->type],stat->name);
				}
			}
		} else {
			// send spot command to server
			strcpy((char *)net_packet->data,"SPOT");
			net_packet->data[4]=player;
			SDLNet_Write32((Uint32)entity->uid,&net_packet->data[5]);
			net_packet->address.host = net_server.host;
			net_packet->address.port = net_server.port;
			net_packet->len = 9;
			sendPacketSafe(net_sock, -1, net_packet, 0);
		}
	}
}