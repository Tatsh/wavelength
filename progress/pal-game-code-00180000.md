# Game code, PAL `0x00180000` to `0x00190000`

191 of 191 routines done.

Sorted by status (remaining first), then length, then reference count, then
name. Names and signatures are the current Ghidra prototypes of the PAL
program and are preliminary. The NTSC-U/C column is empty for a routine only
the PAL executable has.

| Name                                                   |       Status       | # xref | Length | PAL          | NTSC-U/C     | Signature                                                                 |
| ------------------------------------------------------ | :----------------: | -----: | -----: | ------------ | ------------ | ------------------------------------------------------------------------- |
| `NetHostingScreen__Enter`                              | :white_check_mark: |      0 |   5288 | `0x00184850` | `0x00180610` | `undefined NetHostingScreen__EnterGameSetupSelectScreen()`                |
| `NetSearchScreen__OnModeChanged`                       | :white_check_mark: |      0 |   4584 | `0x00188d10` | `0x00184ad0` | `undefined RebuildHostSkillLabelsIncludingAnyEntry()`                     |
| `NetHostingScreen__OnModeChanged`                      | :white_check_mark: |      0 |   3656 | `0x00185cf8` | `0x00181ab8` | `undefined NetHostingScreen__RebuildSkillOptionLabelList()`               |
| `NetHostingScreen__OnChoiceChanged`                    | :white_check_mark: |      0 |   3048 | `0x00186b40` | `0x00182900` | `undefined NetHostingScreen__PopulateHostArenaSelectList()`               |
| `NetSearchScreen__Enter`                               | :white_check_mark: |      0 |   2240 | `0x00188450` | `0x00184210` | `undefined PopulateHostModeOptionList()`                                  |
| `RemixSongScreen__Enter`                               | :white_check_mark: |      0 |   2160 | `0x0018f3b0` | `0x00188e18` | `undefined RemixSongScreen__BuildRemixSongSelectList()`                   |
| `NetSearchScreen__OnChoiceChanged`                     | :white_check_mark: |      0 |   1664 | `0x00189ef8` | `0x00185cb8` | `undefined PopulateArenaSearchSelectList()`                               |
| `FUN_0018e808`                                         | :white_check_mark: |      1 |   1652 | `0x0018e808` |              | `undefined FUN_0018e808()`                                                |
| `PlayerSearchResultsScreen__Enter`                     | :white_check_mark: |      0 |   1224 | `0x00181b38` | `0x0017de78` | `undefined PlayerSearchResultsScreen__EnterFoundPlayerScreen()`           |
| `FUN_0018de98`                                         | :white_check_mark: |      1 |   1168 | `0x0018de98` |              | `undefined FUN_0018de98()`                                                |
| `FUN_001839d8`                                         | :white_check_mark: |      1 |   1096 | `0x001839d8` |              | `undefined FUN_001839d8()`                                                |
| `NetHostingScreen__HandleSelect`                       | :white_check_mark: |      1 |   1060 | `0x00187980` | `0x00183740` | `undefined HandleHostSetupConfirmSelect()`                                |
| `RemixSelectScreen__LoadSelectedRemix`                 | :white_check_mark: |      2 |   1008 | `0x0018ff88` | `0x001899f0` | `undefined CommitSelectedRemixLoad()`                                     |
| `ShareRemixScreen__HandleProgress`                     | :white_check_mark: |      2 |    868 | `0x0018c688` | `0x00188448` | `undefined HandleShareRemixProgressMsg()`                                 |
| `FUN_0018d6a8`                                         | :white_check_mark: |      3 |    848 | `0x0018d6a8` |              | `undefined FUN_0018d6a8()`                                                |
| `FUN_0018dc30`                                         | :white_check_mark: |      1 |    616 | `0x0018dc30` |              | `undefined FUN_0018dc30()`                                                |
| `NetSearchScreen__HandleSelect`                        | :white_check_mark: |      1 |    608 | `0x0018a5e0` | `0x001863a0` | `undefined HandleArenaSearchConfirmSelect()`                              |
| `RemixTypeScreen__HandleSelect`                        | :white_check_mark: |      1 |    588 | `0x0018f110` | `0x00188b78` | `undefined HandleRemixMenuSaveCommand()`                                  |
| `NetLpadScreen__Poll`                                  | :white_check_mark: |      0 |    568 | `0x0018b948` | `0x00187708` | `undefined NetLpadScreen__PollNetLaunchpadHostPlayerList()`               |
| `NetDoPlayerSearchScreen__HandleFindPlayer`            | :white_check_mark: |      1 |    560 | `0x00181898` | `0x0017dbd8` | `undefined HandlePlayerSearchResultResponse()`                            |
| `UploadNoteScreen__HandleJoypad`                       | :white_check_mark: |      1 |    528 | `0x001829e0` | `0x0017ecf8` | `undefined HandleLoadRemixBackOrFocus()`                                  |
| `NetParamsScreen__HandleSelectStart`                   | :white_check_mark: |      2 |    520 | `0x00184448` | `0x00180208` | `undefined HandleSetupListNavigate()`                                     |
| `FUN_0018d140`                                         | :white_check_mark: |      1 |    512 | `0x0018d140` |              | `undefined FUN_0018d140()`                                                |
| `NetHostAttemptScreen__HandleJoypad`                   | :white_check_mark: |      1 |    504 | `0x00188220` | `0x00183fe0` | `undefined HandleHostLaunchpadBackExit()`                                 |
| `NetJoinLPadScreen__HandleJoypad`                      | :white_check_mark: |      1 |    500 | `0x0018b2a0` | `0x00187060` | `undefined HandleNetSwitchLobbyBackExit()`                                |
| `NetMainScreen__HandleJoypad`                          | :white_check_mark: |      1 |    480 | `0x00180a00` | `0x0017cda8` | `undefined HandleMainMenuNavigationEvent()`                               |
| `NetLpadScreen__HandleSelect`                          | :white_check_mark: |      1 |    476 | `0x0018bfb8` | `0x00187d78` | `undefined HandleHostLaunchpadCommandSelect()`                            |
| `RemixSongScreen__HandleSelect`                        | :white_check_mark: |      1 |    444 | `0x0018fc88` | `0x001896f0` | `undefined HandleRemixSongSelectSaveCommand()`                            |
| `SetupNameScreen__HandleJoypad`                        | :white_check_mark: |      1 |    444 | `0x00180f28` | `0x0017d2d0` | `undefined HandleNameEntryKeyboardOpen()`                                 |
| `NetNewChatroomScreen__HandleJoinChatroomResult`       | :white_check_mark: |      1 |    440 | `0x001814c8` | `0x0017d808` | `undefined HandleCreateLobbyServerResponse()`                             |
| `UploadNoteScreen__HandleSelect`                       | :white_check_mark: |      1 |    436 | `0x00182bf0` | `0x0017ef08` | `undefined FUN_00182bf0()`                                                |
| `NetHostingScreen__HandleSelectStart`                  | :white_check_mark: |      1 |    420 | `0x00187da8` | `0x00183b68` | `undefined HandleHostSetupSpinnerNavigate()`                              |
| `NetHostingScreen__UpdateLabels`                       | :white_check_mark: |      0 |    420 | `0x00187728` | `0x001834e8` | `undefined NetHostingScreen__RefreshHostPowerupAndPlayerWidgets()`        |
| `NetMainScreen__HandleSelect`                          | :white_check_mark: |      1 |    404 | `0x00180750` | `0x0017caf8` | `undefined HandleMainMenuScreenCommand()`                                 |
| `NetJoinLPadScreen__HandleAborted`                     | :white_check_mark: |      1 |    396 | `0x0018b110` | `0x00186ed0` | `undefined HandleNetLobbyJoinAbortOrError()`                              |
| `SelRemixUploadScreen__HandleSelect`                   | :white_check_mark: |      1 |    392 | `0x00182620` | `0x0017e960` | `undefined HandleRankLoadRemixSelect()`                                   |
| `FUN_0018cf40`                                         | :white_check_mark: |      1 |    372 | `0x0018cf40` |              | `undefined FUN_0018cf40()`                                                |
| `NetRankingScreen__RequestPage`                        | :white_check_mark: |      2 |    360 | `0x00182330` | `0x0017e670` | `undefined RefreshRankSongList()`                                         |
| `SelRemixDownloadScreen__HandleSelect`                 | :white_check_mark: |      1 |    356 | `0x00183568` | `0x0017f7f0` | `undefined FUN_00183568()`                                                |
| `NetJoinLPadScreen__ShowLaunchpadLost`                 | :white_check_mark: |      7 |    336 | `0x0018b498` | `0x00187258` | `undefined CheckLobbyErrorTransitionState()`                              |
| `NetParamsScreen__UpdateLabels`                        | :white_check_mark: |      1 |    336 | `0x00184148` | `0x0017ff08` | `undefined NetParamsScreen__RefreshPanelListSelectionLabels()`            |
| `FUN_0018d340`                                         | :white_check_mark: |      1 |    312 | `0x0018d340` |              | `undefined FUN_0018d340()`                                                |
| `SetupNewChatroomScreen__Submit`                       | :white_check_mark: |      0 |    304 | `0x001811a8` | `0x0017d558` | `undefined FUN_001811a8()`                                                |
| `NetDoDownloadScreen__HandleRepoRemixStatus`           | :white_check_mark: |      1 |    300 | `0x00183e20` | `0x0017fc20` | `undefined FUN_00183e20()`                                                |
| `NetLpadScreen__DispatchPriv`                          | :white_check_mark: |      0 |    300 | `0x0018bb80` | `0x00187940` | `undefined NetLpadScreen__RouteNetServerLoginScreenMessage()`             |
| `NetMainScreen__DispatchPriv`                          | :white_check_mark: |      0 |    300 | `0x00180580` | `0x0017c928` | `undefined NetMainScreen__DispatchNetMainScreenMessage()`                 |
| `NetRankingScreen__Enter`                              | :white_check_mark: |      0 |    296 | `0x001821a0` | `0x0017e4e0` | `undefined NetRankingScreen__EnterRankSongListScreen()`                   |
| `NetParamsScreen__HandleJoypad`                        | :white_check_mark: |      2 |    280 | `0x00184650` | `0x00180410` | `undefined HandleSetupListBack()`                                         |
| `FUN_0018e618`                                         | :white_check_mark: |      1 |    280 | `0x0018e618` |              | `undefined FUN_0018e618()`                                                |
| `NetMainScreen__HandleSelectStart`                     | :white_check_mark: |      1 |    276 | `0x001808e8` | `0x0017cc90` | `undefined HandleMainMenuDeselectCommand()`                               |
| `NetMainScreen__Enter`                                 | :white_check_mark: |      0 |    276 | `0x00180350` | `0x0017c6f8` | `undefined NetMainScreen__ActivateNetMainScreen()`                        |
| `NetGetDownloadsScreen__HandleRepoRemixes`             | :white_check_mark: |      1 |    256 | `0x001832d8` | `0x0017f560` | `undefined HandleDownloadListResult()`                                    |
| `NetLpadScreen__Enter`                                 | :white_check_mark: |      0 |    256 | `0x0018b6a8` | `0x00187468` | `undefined NetLpadScreen__HandleNetLaunchpadHostScreenExit()`             |
| `NetParamsScreen__HandleSelect`                        | :white_check_mark: |      3 |    252 | `0x00184348` | `0x00180108` | `undefined HandleSetupListSelect()`                                       |
| `NetSwitchLobbyScreen__HandleJoinResult`               | :white_check_mark: |      1 |    252 | `0x0018ad90` | `0x00186b50` | `undefined HandleNetChatroomSelect()`                                     |
| `NetLpadScreen__HandleJoypad`                          | :white_check_mark: |      1 |    248 | `0x0018bcb0` | `0x00187a70` | `undefined ConfirmExitNetServerLoginScreen()`                             |
| `NetNewChatroomScreen__HandleTransitionComplete`       | :white_check_mark: |      1 |    244 | `0x001813d0` | `0x0017d710` | `undefined HandleChatroomFocusEvent()`                                    |
| `FUN_0018e4c0`                                         | :white_check_mark: |      0 |    240 | `0x0018e4c0` |              | `undefined FUN_0018e4c0()`                                                |
| `PlayerSearchResultsScreen__HandleSelect`              | :white_check_mark: |      1 |    236 | `0x00182068` | `0x0017e3a8` | `undefined HandleSwitchLobbyMenuCommand()`                                |
| `NetHostingScreen__NetHostingScreen`                   | :white_check_mark: |      1 |    232 | `0x00184768` | `0x00180528` | `undefined InitGameSetupSelectScreenObject()`                             |
| `NetRankingScreen__HandleLobbyPlayers`                 | :white_check_mark: |      1 |    232 | `0x00182498` | `0x0017e7d8` | `undefined HandleRankPlayerSelect()`                                      |
| `SetupNameScreen__Enter`                               | :white_check_mark: |      1 |    232 | `0x00180d90` | `0x0017d138` | `undefined BindNameEntryTextWidget()`                                     |
| `FUN_0018e3e0`                                         | :white_check_mark: |      0 |    220 | `0x0018e3e0` |              | `undefined FUN_0018e3e0()`                                                |
| `BootScreen__HandleSelect`                             | :white_check_mark: |      1 |    216 | `0x0018c4a8` | `0x00188268` | `undefined HandleNetQuitConfirmYesNo()`                                   |
| `FUN_0018e730`                                         | :white_check_mark: |      1 |    216 | `0x0018e730` |              | `undefined FUN_0018e730()`                                                |
| `UploadNoteScreen__HandleFocusChange`                  | :white_check_mark: |      1 |    216 | `0x00182da8` | `0x0017f030` | `undefined HandleLoadRemixFocusChange()`                                  |
| `NetMainScreen__Exit`                                  | :white_check_mark: |      0 |    216 | `0x00180468` | `0x0017c810` | `undefined NetMainScreen__LeaveNetMainScreen()`                           |
| `NetLpadScreen__Exit`                                  | :white_check_mark: |      0 |    212 | `0x0018b7a8` | `0x00187568` | `undefined NetLpadScreen__HandleNetLaunchpadHostScreenEnter()`            |
| `NetEndGameScreen__HandleTransitionComplete`           | :white_check_mark: |      1 |    204 | `0x0018cc28` | `0x001889e8` | `undefined HandleNetEndgameWaitFocus()`                                   |
| `FUN_0018cd78`                                         | :white_check_mark: |      0 |    204 | `0x0018cd78` |              | `undefined FUN_0018cd78()`                                                |
| `NetHostAttemptScreen__DispatchPriv`                   | :white_check_mark: |      0 |    204 | `0x00188008` | `0x00183dc8` | `undefined NetHostAttemptScreen__DispatchHostLaunchpadMessage()`          |
| `NetJoinLPadScreen__DispatchPriv`                      | :white_check_mark: |      0 |    204 | `0x0018af18` | `0x00186cd8` | `undefined NetJoinLPadScreen__RouteNetSwitchLobbyScreenMessage()`         |
| `UploadNoteScreen__DispatchPriv`                       | :white_check_mark: |      0 |    204 | `0x00182910` | `0x0017ec28` | `undefined UploadNoteScreen__HandleLoadRemixScreenMessage()`              |
| `NetLpadScreen__UpdateGameParams`                      | :white_check_mark: |      1 |    200 | `0x0018b880` | `0x00187640` | `undefined RefreshNetHostGamePanelOnFocus()`                              |
| `NetParamsScreen__Exit`                                | :white_check_mark: |      0 |    200 | `0x00184080` | `0x0017fe40` | `undefined ClearPanelListScreenWidgets()`                                 |
| `FUN_0018d550`                                         | :white_check_mark: |      1 |    196 | `0x0018d550` |              | `undefined FUN_0018d550()`                                                |
| `NetLpadScreen__NetLpadScreen`                         | :white_check_mark: |      1 |    188 | `0x0018b5e8` | `0x001873a8` | `undefined InitNetLaunchpadHostScreenWithPlayerList()`                    |
| `PlayerSearchSetupScreen__Submit`                      | :white_check_mark: |      0 |    188 | `0x00181680` | `0x0017d9c0` | `undefined PlayerSearchSetupScreen__OpenPlayerSearchScreenWithText()`     |
| `NetParamsScreen__NetParamsScreen`                     | :white_check_mark: |      2 |    184 | `0x00183f50` | `0x0017fd10` | `undefined InitPanelListScreenObject()`                                   |
| `NetLpadScreen__HandleFocusChange`                     | :white_check_mark: |      1 |    184 | `0x0018be48` | `0x00187c08` | `undefined HandleNetServerLoginSaveFocus()`                               |
| `NetLpadScreen__HandleSelectStart`                     | :white_check_mark: |      1 |    184 | `0x0018bf00` | `0x00187cc0` | `undefined SyncHostLaunchpadWidgetOnMenuChange()`                         |
| `NetDoDownloadScreen__ShowProgress`                    | :white_check_mark: |      2 |    180 | `0x00183920` | `0x0017fb68` | `undefined UpdateDownloadProgressDialog()`                                |
| `NetDoUploadScreen__ShowProgress`                      | :white_check_mark: |      2 |    180 | `0x00182f58` | `0x0017f1e0` | `undefined UpdateUploadProgressDialog()`                                  |
| `FUN_0018d9f8`                                         | :white_check_mark: |      1 |    180 | `0x0018d9f8` |              | `undefined FUN_0018d9f8()`                                                |
| `FUN_0018dab0`                                         | :white_check_mark: |      1 |    180 | `0x0018dab0` |              | `undefined FUN_0018dab0()`                                                |
| `FUN_0018db68`                                         | :white_check_mark: |      1 |    180 | `0x0018db68` |              | `undefined FUN_0018db68()`                                                |
| `FUN_0018e328`                                         | :white_check_mark: |      1 |    180 | `0x0018e328` |              | `undefined FUN_0018e328()`                                                |
| `NetEndGameScreen__Poll`                               | :white_check_mark: |      0 |    180 | `0x0018ca88` | `0x00188848` | `undefined NetEndGameScreen__UpdateNetEndgameWaitTimeout()`               |
| `RemixSelectScreen__Enter`                             | :white_check_mark: |      0 |    180 | `0x0018fed0` | `0x00189938` | `undefined RemixSelectScreen__BuildRemixLoadList()`                       |
| `NetJoinLPadScreen__HandleSuccess`                     | :white_check_mark: |      1 |    176 | `0x0018b060` | `0x00186e20` | `undefined HandleNetLobbyJoinSuccess()`                                   |
| `UploadNoteScreen__Enter`                              | :white_check_mark: |      0 |    176 | `0x00182840` | `0x0017eb58` | `undefined UploadNoteScreen__EnterLoadRemixScreen()`                      |
| `NetParamsScreen__DispatchPriv`                        | :white_check_mark: |      2 |    172 | `0x00184298` | `0x00180058` | `undefined NetParamsScreen__HandleSetupListScreenMessage()`               |
| `NetSortedScreen__NetSortedScreen`                     | :white_check_mark: |      1 |    172 | `0x0018a840` | `0x00186600` | `undefined InitNetLaunchpadInfoScreen()`                                  |
| `NetDoDownloadScreen__DispatchPriv`                    | :white_check_mark: |      0 |    172 | `0x00183800` | `0x0017fa68` | `undefined NetDoDownloadScreen__HandleDoDownloadScreenMessage()`          |
| `NetHostingScreen__DispatchPriv`                       | :white_check_mark: |      0 |    172 | `0x001878d0` | `0x00183690` | `undefined NetHostingScreen__DispatchHostSetupListMessage()`              |
| `SelRemixDownloadScreen__HandleJoypad`                 | :white_check_mark: |      1 |    168 | `0x001836d0` | `0x0017f940` | `undefined HandleSelRemixDownloadPreview()`                               |
| `NetHostAttemptScreen__HandleSuccess`                  | :white_check_mark: |      1 |    164 | `0x00188128` | `0x00183ee8` | `undefined HandleHostLaunchpadStart()`                                    |
| `NetDoUploadScreen__HandleRepoRemixStatus`             | :white_check_mark: |      1 |    160 | `0x00183110` | `0x0017f398` | `undefined HandleUploadResultMessage()`                                   |
| `NetMainScreen__HandleFocusChange`                     | :white_check_mark: |      1 |    160 | `0x001806b0` | `0x0017ca58` | `undefined HandleNetMainTabSelect()`                                      |
| `NetMainScreen__HandleLobbyPlayers`                    | :white_check_mark: |      1 |    160 | `0x00180c80` | `0x0017d028` | `undefined HandleJoinGameRankUpdateEvent()`                               |
| `BootScreen__Enter`                                    | :white_check_mark: |      0 |    156 | `0x0018c3a0` | `0x00188160` | `undefined BootScreen__PopulateNetQuitConfirmDialogText()`                |
| `NetSortedScreen__Enter`                               | :white_check_mark: |      0 |    156 | `0x0018a8f0` | `0x001866b0` | `undefined NetSortedScreen__EnterSortedResultsScreenWithTile()`           |
| `UploadNoteScreen__UploadNoteScreen`                   | :white_check_mark: |      1 |    152 | `0x001827a8` | `0x0017eae8` | `undefined FUN_001827a8()`                                                |
| `NetSwitchLobbyScreen__HandleTransitionComplete`       | :white_check_mark: |      1 |    148 | `0x0018acf8` | `0x00186ab8` | `undefined HandleNetChatroomScreenFocusTransition()`                      |
| `NetLaunchScreen__HandleTransitionComplete`            | :white_check_mark: |      1 |    144 | `0x0018c2a8` | `0x00188068` | `undefined HandleNetShareRemixScreenFocus()`                              |
| `NetEULAScreen__HandleJoypad`                          | :white_check_mark: |      1 |    140 | `0x00180238` | `0x0017c5e0` | `undefined HandleEulaScrollNavigation()`                                  |
| `FUN_0018d618`                                         | :white_check_mark: |      0 |    140 | `0x0018d618` |              | `undefined FUN_0018d618()`                                                |
| `FUN_0018ef00`                                         | :white_check_mark: |      0 |    140 | `0x0018ef00` |              | `undefined FUN_0018ef00()`                                                |
| `NetDoPlayerSearchScreen__DispatchPriv`                | :white_check_mark: |      0 |    140 | `0x001817a8` | `0x0017dae8` | `undefined NetDoPlayerSearchScreen__DispatchPlayerSearchScreenEvent()`    |
| `NetDoUploadScreen__DispatchPriv`                      | :white_check_mark: |      0 |    140 | `0x00183010` | `0x0017f298` | `undefined NetDoUploadScreen__HandleDoUploadScreenMessage()`              |
| `NetEndGameScreen__DispatchPriv`                       | :white_check_mark: |      0 |    140 | `0x0018cb40` | `0x00188900` | `undefined NetEndGameScreen__DispatchNetEndgameWaitScreenEvent()`         |
| `NetGetDownloadsScreen__DispatchPriv`                  | :white_check_mark: |      0 |    140 | `0x001831e8` | `0x0017f470` | `undefined NetGetDownloadsScreen__HandleGetDownloadListScreenMessage()`   |
| `NetLaunchpadQuitScreen__DispatchPriv`                 | :white_check_mark: |      0 |    140 | `0x0018aaf0` | `0x001868b0` | `undefined NetLaunchpadQuitScreen__RouteNetLaunchpadScreenMessage()`      |
| `NetNewChatroomScreen__DispatchPriv`                   | :white_check_mark: |      0 |    140 | `0x00181340` | `0x0017d680` | `undefined NetNewChatroomScreen__DispatchChatroomScreenEvent()`           |
| `NetSwitchLobbyScreen__DispatchPriv`                   | :white_check_mark: |      0 |    140 | `0x0018ac68` | `0x00186a28` | `undefined NetSwitchLobbyScreen__RouteNetChatroomScreenMessage()`         |
| `SelRemixDownloadScreen__DispatchPriv`                 | :white_check_mark: |      0 |    140 | `0x001834d8` | `0x0017f760` | `undefined SelRemixDownloadScreen__HandleSelRemixDownloadScreenMessage()` |
| `SetupNameScreen__DispatchPriv`                        | :white_check_mark: |      0 |    140 | `0x00180e98` | `0x0017d240` | `undefined DispatchNameEntryScreenEvent()`                                |
| `SetupNewChatroomScreen__Enter`                        | :white_check_mark: |      0 |    140 | `0x00181118` | `0x0017d4c8` | `undefined SetupNewChatroomScreen__SeedNameEntryWidgetInitialChar()`      |
| `ShareRemixScreen__DispatchPriv`                       | :white_check_mark: |      0 |    140 | `0x0018c5f8` | `0x001883b8` | `undefined ShareRemixScreen__DispatchRemixSaveScreenEvent()`              |
| `NetMainScreen__NetMainScreen`                         | :white_check_mark: |      1 |    136 | `0x001802c8` | `0x0017c670` | `undefined InitL1PanelScreenState()`                                      |
| `RemixSelectScreen__RemixSelectScreen`                 | :white_check_mark: |      3 |    132 | `0x0018fe48` | `0x001898b0` | `undefined InitRemixLoadListScreen()`                                     |
| `FUN_0018d0b8`                                         | :white_check_mark: |      1 |    132 | `0x0018d0b8` |              | `undefined FUN_0018d0b8()`                                                |
| `NetDoDownloadScreen__NetDoDownloadScreen`             | :white_check_mark: |      1 |    132 | `0x00183778` | `0x0017f9e8` | `undefined InitDoDownloadScreenObject()`                                  |
| `FUN_0018ccf8`                                         | :white_check_mark: |      1 |    128 | `0x0018ccf8` |              | `undefined FUN_0018ccf8()`                                                |
| `FUN_0018ee80`                                         | :white_check_mark: |      1 |    128 | `0x0018ee80` |              | `undefined FUN_0018ee80()`                                                |
| `NetHostingScreen__HandleJoypad`                       | :white_check_mark: |      1 |    128 | `0x00187f50` | `0x00183d10` | `undefined HandleHostSetupBackExit()`                                     |
| `SelRemixDownloadScreen__SelRemixDownloadScreen`       | :white_check_mark: |      1 |    128 | `0x001833d8` | `0x0017f660` | `undefined InitSelRemixDownloadScreenObject()`                            |
| `NetLaunchpadQuitScreen__HandleTransitionComplete`     | :white_check_mark: |      1 |    124 | `0x0018ab80` | `0x00186940` | `undefined HandleNetLaunchpadScreenFocusTransition()`                     |
| `SelRemixDownloadScreen__Enter`                        | :white_check_mark: |      0 |    124 | `0x00183458` | `0x0017f6e0` | `undefined SelRemixDownloadScreen__EnterSelRemixDownloadScreen()`         |
| `NetParamsScreen__Enter`                               | :white_check_mark: |      2 |    120 | `0x00184008` | `0x0017fdc8` | `undefined NetParamsScreen__EnterPanelListScreen()`                       |
| `NetSortedScreen__HandleLobbyPlayers`                  | :white_check_mark: |      1 |    120 | `0x0018aa40` | `0x00186800` | `undefined HandlePlayerSelectSortedPicMessage()`                          |
| `NetJoinLPadScreen__HandleTransitionComplete`          | :white_check_mark: |      1 |    116 | `0x0018afe8` | `0x00186da8` | `undefined HandleNetSwitchLobbyFocus()`                                   |
| `NetJoinLPadScreen__NetJoinLPadScreen`                 | :white_check_mark: |      1 |    116 | `0x0018ae90` | `0x00186c50` | `undefined InitNetSwitchLobbyScreenVtable()`                              |
| `ShareRemixScreen__ShareRemixScreen`                   | :white_check_mark: |      1 |    116 | `0x0018c580` | `0x00188340` | `undefined InitNetLaunchScreenVtableAndStream()`                          |
| `SetupNameScreen__SetupNameScreen`                     | :white_check_mark: |      2 |    112 | `0x00180d20` | `0x0017d0c8` | `undefined InitNameEntryScreenObject()`                                   |
| `UploadNoteScreen__HandleTextEntryComplete`            | :white_check_mark: |      1 |    112 | `0x00182e80` | `0x0017f108` | `undefined SubmitLoadRemixServerLogin()`                                  |
| `FUN_0018d4e0`                                         | :white_check_mark: |      0 |    112 | `0x0018d4e0` |              | `undefined FUN_0018d4e0()`                                                |
| `NetDoDownloadScreen__HandleTransitionComplete`        | :white_check_mark: |      1 |    108 | `0x001838b0` | `0x0017faf8` | `undefined HandleDoDownloadFocus()`                                       |
| `NetDoUploadScreen__HandleTransitionComplete`          | :white_check_mark: |      1 |    108 | `0x001830a0` | `0x0017f328` | `undefined HandleDoUploadFocus()`                                         |
| `NetLpadScreen__HandleTransitionComplete`              | :white_check_mark: |      1 |    108 | `0x0018c198` | `0x00187f58` | `undefined NotifySessionOnHostLaunchpadFocusGained()`                     |
| `PlayerSearchResultsScreen__PlayerSearchResultsScreen` | :white_check_mark: |      1 |    108 | `0x00181ac8` | `0x0017de08` | `undefined InitFoundPlayerScreenObject()`                                 |
| `BootScreen__DispatchPriv`                             | :white_check_mark: |      0 |    104 | `0x0018c440` | `0x00188200` | `undefined BootScreen__RouteNetQuitConfirmScreenMessage()`                |
| `FUN_0018e5b0`                                         | :white_check_mark: |      0 |    104 | `0x0018e5b0` |              | `undefined FUN_0018e5b0()`                                                |
| `NetEULAScreen__DispatchPriv`                          | :white_check_mark: |      0 |    104 | `0x001801d0` | `0x0017c578` | `undefined NetEULAScreen__DispatchEulaScreenMessage()`                    |
| `NetLaunchScreen__DispatchPriv`                        | :white_check_mark: |      0 |    104 | `0x0018c240` | `0x00188000` | `undefined NetLaunchScreen__RouteNetShareRemixScreenMessage()`            |
| `NetRankingScreen__DispatchPriv`                       | :white_check_mark: |      0 |    104 | `0x001822c8` | `0x0017e608` | `undefined NetRankingScreen__HandleRankScreenMessage()`                   |
| `NetSearchScreen__DispatchPriv`                        | :white_check_mark: |      0 |    104 | `0x0018a578` | `0x00186338` | `undefined DispatchArenaSearchListMessage()`                              |
| `NetSortedScreen__DispatchPriv`                        | :white_check_mark: |      0 |    104 | `0x0018a9d8` | `0x00186798` | `undefined NetSortedScreen__RoutePlayerSelectScreenMessage()`             |
| `PlayerSearchResultsScreen__DispatchPriv`              | :white_check_mark: |      0 |    104 | `0x00182000` | `0x0017e340` | `undefined PlayerSearchResultsScreen__DispatchSwitchLobbyScreenEvent()`   |
| `RemixSongScreen__DispatchPriv`                        | :white_check_mark: |      0 |    104 | `0x0018fc20` | `0x00189688` | `undefined RemixSongScreen__DispatchRemixSongSelectScreenEvent()`         |
| `RemixTypeScreen__DispatchPriv`                        | :white_check_mark: |      0 |    104 | `0x0018f088` | `0x00188af0` | `undefined DispatchRemixMenuScreenEvent()`                                |
| `SelRemixUploadScreen__DispatchPriv`                   | :white_check_mark: |      0 |    104 | `0x001825b8` | `0x0017e8f8` | `undefined SelRemixUploadScreen__HandleRankSaveMessage()`                 |
| `BootScreen__BootScreen`                               | :white_check_mark: |      1 |    100 | `0x0018c338` | `0x001880f8` | `undefined InitNetQuitConfirmScreenVtableAndStream()`                     |
| `NetDoPlayerSearchScreen__NetDoPlayerSearchScreen`     | :white_check_mark: |      1 |    100 | `0x00181740` | `0x0017da80` | `undefined InitPlayerSearchScreenObject()`                                |
| `NetDoUploadScreen__NetDoUploadScreen`                 | :white_check_mark: |      1 |    100 | `0x00182ef0` | `0x0017f178` | `undefined InitDoUploadScreenObject()`                                    |
| `NetNewChatroomScreen__NetNewChatroomScreen`           | :white_check_mark: |      1 |    100 | `0x001812d8` | `0x0017d618` | `undefined InitChatroomScreenObject()`                                    |
| `FUN_0018d478`                                         | :white_check_mark: |      0 |    100 | `0x0018d478` |              | `undefined FUN_0018d478()`                                                |
| `FUN_0018ce48`                                         | :white_check_mark: |      1 |     96 | `0x0018ce48` |              | `undefined FUN_0018ce48()`                                                |
| `FUN_0018ef90`                                         | :white_check_mark: |      1 |     96 | `0x0018ef90` |              | `undefined FUN_0018ef90()`                                                |
| `NetDoPlayerSearchScreen__HandleTransitionComplete`    | :white_check_mark: |      1 |     96 | `0x00181838` | `0x0017db78` | `undefined HandlePlayerSearchFocusEvent()`                                |
| `ShareRemixScreen__HandleTransitionComplete`           | :white_check_mark: |      1 |     96 | `0x0018c9f0` | `0x001887b0` | `undefined HandleRemixSaveScreenFocus()`                                  |
| `FUN_0018eff0`                                         | :white_check_mark: |      1 |     92 | `0x0018eff0` |              | `undefined FUN_0018eff0()`                                                |
| `NetGetDownloadsScreen__HandleTransitionComplete`      | :white_check_mark: |      1 |     92 | `0x00183278` | `0x0017f500` | `undefined HandleGetDownloadListFocus()`                                  |
| `NetEndGameScreen__HandleSelect`                       | :white_check_mark: |      1 |     88 | `0x0018cbd0` | `0x00188990` | `undefined HandleNetEndgameWaitSaveCmd()`                                 |
| `NetLpadScreen__HandleKeyboardKey`                     | :white_check_mark: |      1 |     84 | `0x0018bda8` | `0x00187b68` | `undefined ForwardKeyMessageToHostLaunchpad()`                            |
| `NetMainScreen__HandleKeyboardKey`                     | :white_check_mark: |      1 |     84 | `0x00180be0` | `0x0017cf88` | `undefined ForwardEventToMainContentScreen()`                             |
| `FUN_0018cef0`                                         | :white_check_mark: |      1 |     80 | `0x0018cef0` |              | `undefined FUN_0018cef0()`                                                |
| `NetHostAttemptScreen__HandleTransitionComplete`       | :white_check_mark: |      1 |     80 | `0x001880d8` | `0x00183e98` | `undefined HandleHostLaunchpadFocus()`                                    |
| `NetHostAttemptScreen__HandleAborted`                  | :white_check_mark: |      1 |     76 | `0x001881d0` | `0x00183f90` | `undefined HandleHostLaunchpadAbort()`                                    |
| `RemixSongScreen__RemixSongScreen`                     | :white_check_mark: |      1 |     76 | `0x0018f360` | `0x00188dc8` | `undefined InitRemixSongSelectScreen()`                                   |
| `NetSortedScreen__SetSearch`                           | :white_check_mark: |      1 |     72 | `0x0018a990` | `0x00186750` | `undefined SetSortedScreenSearchParams()`                                 |
| `FUN_0018cea8`                                         | :white_check_mark: |      1 |     68 | `0x0018cea8` |              | `undefined FUN_0018cea8()`                                                |
| `NetLpadScreen__HandleTextEntryComplete`               | :white_check_mark: |      1 |     68 | `0x0018be00` | `0x00187bc0` | `undefined UpdateHostLaunchpadLoginFieldText()`                           |
| `NetMainScreen__HandleTextEntryComplete`               | :white_check_mark: |      1 |     68 | `0x00180c38` | `0x0017cfe0` | `undefined UpdateMainContentKeyboardField()`                              |
| `NetRankingScreen__NetRankingScreen`                   | :white_check_mark: |      1 |     68 | `0x00182158` | `0x0017e498` | `undefined InitSwitchLobbyScreenObject()`                                 |
| `NetMainScreen__Title`                                 | :white_check_mark: |      0 |     60 | `0x00180540` | `0x0017c8e8` | `undefined NetMainScreen__FormatNetMainTitle()`                           |
| `NetEndGameScreen__NetEndGameScreen`                   | :white_check_mark: |      1 |     56 | `0x0018ca50` | `0x00188810` | `undefined InitNetEndgameWaitScreen()`                                    |
| `NetHostAttemptScreen__NetHostAttemptScreen`           | :white_check_mark: |      1 |     56 | `0x00187fd0` | `0x00183d90` | `undefined InitHostSetupListPanel()`                                      |
| `NetGetDownloadsScreen__NetGetDownloadsScreen`         | :white_check_mark: |      1 |     52 | `0x001831b0` | `0x0017f438` | `undefined SetGetDownloadListScreenVtable()`                              |
| `NetLaunchScreen__NetLaunchScreen`                     | :white_check_mark: |      1 |     52 | `0x0018c208` | `0x00187fc8` | `undefined InitNetShareRemixScreenVtable()`                               |
| `NetLaunchpadQuitScreen__NetLaunchpadQuitScreen`       | :white_check_mark: |      1 |     52 | `0x0018aab8` | `0x00186878` | `undefined InitNetLaunchpadScreenVtable()`                                |
| `NetSearchScreen__NetSearchScreen`                     | :white_check_mark: |      1 |     52 | `0x00188418` | `0x001841d8` | `undefined InitHostModeSelectPanel()`                                     |
| `NetSwitchLobbyScreen__NetSwitchLobbyScreen`           | :white_check_mark: |      1 |     52 | `0x0018ac30` | `0x001869f0` | `undefined InitNetChatroomScreenVtable()`                                 |
| `RemixTypeScreen__RemixTypeScreen`                     | :white_check_mark: |      1 |     52 | `0x0018f050` | `0x00188ab8` | `undefined ConstructNetLobbyListNode()`                                   |
| `SelRemixUploadScreen__SelRemixUploadScreen`           | :white_check_mark: |      1 |     52 | `0x00182580` | `0x0017e8c0` | `undefined SetRankScreenSaveVtable()`                                     |
| `NetLaunchpadQuitScreen__HandleAborted`                | :white_check_mark: |      1 |     44 | `0x0018ac00` | `0x001869c0` | `undefined HandleNetLaunchpadAbort()`                                     |
| `SetupNameScreen__HandleTextEntryComplete`             | :white_check_mark: |      1 |     44 | `0x001810e8` | `0x0017d498` | `undefined InvokeNameEntrySubmit()`                                       |
| `SetupNameScreen__ReceiveKeyboardText`                 | :white_check_mark: |      0 |     32 | `0x00180e78` | `0x0017d220` | `undefined ClearNameEntryFieldString()`                                   |
| `UploadNoteScreen__ReceiveKeyboardText`                | :white_check_mark: |      0 |     32 | `0x001828f0` | `0x0017ec08` | `undefined UploadNoteScreen__ExitLoadRemixScreen()`                       |
| `RemixTypeScreen__Enter`                               | :white_check_mark: |      0 |     28 | `0x0018f0f0` | `0x00188b58` | `undefined ExitRemixMenuScreen()`                                         |
| `NetJoinLPadScreen__SetLaunchpad`                      | :white_check_mark: |      1 |     12 | `0x0018af08` | `0x00186cc8` | `undefined SetNetSwitchLobbyScreenSeedFields()`                           |
