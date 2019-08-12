#include <chrono>
#include <iostream>

#include "main_window.h"
#include "game.h"
#include "move.h"

// 128ツモの初期化
void Game::initTumo() {
    Color except_color = randomColor();

    for (int i = 0; i < TUMO_MAX; i++) {
        tumos[i] = Tumo(3, 12, randomColor(except_color), randomColor(except_color), 0);
    }
}

void Game::init() {
    p1.init();
    p2.init();
    
    // リプレイ情報の初期化
    if (!replay_mode) {
        initTumo();
    }

    // リプレイ情報からツモ再生
    else {
        for (int i = 0; i < TUMO_MAX; i++) {
            tumos[i] = battle_history.tumo_history[i];
        }
    }

    start_time = now();
}

void Game::stop() {
    stop_time = now();
}

void Game::restart() {
    start_time += now() - stop_time;
}

void Game::update() {
    GameFase bef1 = p1.field.getGamefase();
    GameFase bef2 = p2.field.getGamefase();
    p1.field.update(p2.field, getKeyOperation(p1.status));
    p2.field.update(p1.field, getKeyOperation(p2.status));
    GameFase gf1 = p1.field.getGamefase();
    GameFase gf2 = p2.field.getGamefase();
    onFase(bef1, gf1, p1, p1.field.getChain());
    onFase(bef2, gf2, p2, p2.field.getChain());
    show();
}

void Game::onFase(GameFase bf, GameFase gf, Player& player, int chain) {
    // ゲームオーバー時の処理
    if (gf == GAMEOVER) {
        bool p1_win = (player.status & PLAYER1) ? false : true;

        if (p1.status & PLAYER_AI) {
            p1.upi.gameover(p1_win);
        }
        if (p2.status & PLAYER_AI) {
            p2.upi.gameover(!p1_win);
        }

        p1_win ? p1.win++ : p2.win++;

        if (!replay_mode) {
            std::string replay_file_name = "";
            replay_file_name += battle_history.player_1p_name + "_vs_" + battle_history.player_2p_name + "_" + timeStamp() + ".puyofu";
            battle_history.save("replay/" + replay_file_name);
        }
        
        main_window->gameOver();
        replay_mode = false;
    }

    // engineに考えさせる
    else if (bf == NEXT) {
        if (!replay_mode) {
            if (player.status & PLAYER_AI) {
                if (player.status & PLAYER1) {
                    p1.upi.setEngineMove(p1.field, p2.field, p1.operation);
                }
                else {
                    p2.upi.setEngineMove(p2.field, p1.field, p2.operation);
                }
            }
        }
    }

    // 連鎖ボイス再生
    else if (gf == CHAIN_VOICE) {
        if (chain >= 11) {            
            main_window->playVoice(player.status & PLAYER1 ? 0 : 1, 10);
            player.frame++;            
        }
        else {        
            main_window->playVoice(player.status & PLAYER1 ? 0 : 1, chain - 1);
        }        
    }

    // 連鎖ボイスの10と1~9を組み合わせて「11」「19」を言う
    if (player.frame) {
        player.frame++;
        if (player.frame > 30) {
            main_window->playVoice(player.status & PLAYER1 ? 0 : 1, chain - 11);
            player.frame = 0;
        }
    }
}

// 操作の注意点
// 横移動は、右、左どちらかの移動のみ受け付ける
// 移動しながらの回転は認めるが、回転押しっぱなしの横移動は認めない
// その場合は、移動を優先することにする（回転はしない）
// また、フィールド上で回転できない場所は存在しないものとする
// 回転キーおしっぱなしで横、↓移動の場合は回転がキャンセルされ、移動する
// 移動し終わって回転キーが押しっぱなしでも回転しない。（一度押しなおすと作動する)
OperationBit Game::getKeyOperation(PlayerStatus status) {
    OperationBit operation = NO_OPERATION;

    if (replay_mode) {
        if (status & PLAYER1) {
            operation = battle_history.move_history_1p.front();
            battle_history.move_history_1p.pop_front();
        }
        else {
            operation = battle_history.move_history_2p.front();
            battle_history.move_history_2p.pop_front();
        }
    }
    else {
        if (status & PLAYER_AI) {
            if (status & PLAYER1) {
                if (p1.operation.size()) {
                    operation = p1.operation.pop();
                }
            }
            else {
                if (p2.operation.size()) {
                    operation = p2.operation.pop();
                }
            }
        }
        else {
            int keys[2][5] = {
                { VK_LEFT, VK_RIGHT, VK_M, VK_N, VK_DOWN },
                { VK_A, VK_D, VK_G, VK_F, VK_S }
            };

            int p = (status & PLAYER1) ? 0 : 1;
            if (GetAsyncKeyState(keys[p][0])) {
                operation |= OPE_LEFT;
            }
            else if (GetAsyncKeyState(keys[p][1])) {
                operation |= OPE_RIGHT;
            }
            if (GetAsyncKeyState(keys[p][2])) {
                operation |= OPE_R_ROTATE;
            }
            else if (GetAsyncKeyState(keys[p][3])) {
                operation |= OPE_L_ROTATE;
            }
            if (GetAsyncKeyState(keys[p][4])) {
                operation |= OPE_DOWN;
            }
        }

        if (status & PLAYER1) {
            battle_history.move_history_1p.push_back(operation);
        }
        else {
            battle_history.move_history_2p.push_back(operation);
        }
    }

    return operation;
}

// 画面の描画
void Game::show() {
    RECT rect_p1_score = { 153, 635, 336, 663 };
    RECT rect_p2_score = { 778, 633, 930, 663 };
    RECT rect_p1_chain = { 373, 294, 408, 400 };
    RECT rect_p2_chain = { 568, 294, 602, 400 };
    RECT rect_p1_win   = { 358, 474, 403, 510 };
    RECT rect_p2_win   = { 584, 474, 624, 510 };
    RECT rect_elapsed  = { 430, 410, 584, 440 };
    TCHAR score1[128], score2[128], rensa1[24], rensa2[24], time[24], p1_win[12], p2_win[12];
    TimePoint elapsed = now() - start_time;
    if (p1.max_chain < p1.field.getChain()) {
        p1.max_chain = p1.field.getChain();
    }
    if (p2.max_chain < p2.field.getChain()) {
        p2.max_chain = p2.field.getChain();
    }
    wsprintf(score1, "%d", p1.field.getScoreSum());
    wsprintf(score2, "%d", p2.field.getScoreSum());                           
    wsprintf(rensa1, "chain\n %d\nmax\n %d", p1.field.getChain(), p1.max_chain);
    wsprintf(rensa2, "chain\n %d\nmax\n %d", p2.field.getChain(), p2.max_chain);
    wsprintf(time, "GameTime:%6ds", (int)(elapsed / 1000));
    wsprintf(p1_win, "%dwin", p1.win);
    wsprintf(p2_win, "%dwin", p2.win);
    p1.field.show(main_window, p1.status);
    p2.field.show(main_window, p2.status);
    main_window->drawString(&rect_p1_score, score1, DT_RIGHT);
    main_window->drawString(&rect_p2_score, score2, DT_RIGHT);
    main_window->drawString(&rect_p1_chain, rensa1, false);
    main_window->drawString(&rect_p2_chain, rensa2, false);
    main_window->drawString(&rect_elapsed,  time,   false);
    main_window->drawString(&rect_p1_win,   p1_win, false);
    main_window->drawString(&rect_p2_win,   p2_win, false);
}
