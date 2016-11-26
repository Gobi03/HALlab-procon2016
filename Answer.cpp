//------------------------------------------------------------------------------
/// @file
/// @author   ハル研究所プログラミングコンテスト実行委員会
///
/// @copyright  Copyright (c) 2016 HAL Laboratory, Inc.
/// @attention  このファイルの利用は、同梱のREADMEにある
///             利用条件に従ってください
//------------------------------------------------------------------------------

// ver2.3

#include "Answer.hpp"

#include <bits/stdc++.h>
#include <iostream>
using namespace std;

#define FOR(i,a,b) for(int i=(a);i<(b);++i)
#define UNSFOR(i,a,b) for(unsigned int i=(a);i<(b);++i)
#define REP(i,n)  FOR(i,0,n)
#define UNSREP(i,n)  UNSFOR(i,0,n)
#define debug(x) cout << #x << " = " << x << endl
#define debugT(x, turn) if(stageNumber == turn)cout << #x << " = " << x << endl
#define pb(x) push_back(x)


/// プロコン問題環境を表します。
namespace hpc {
  /*-------------------------------------*/
  /* variables */
  /*-------------------------------------*/
  /* evaluation */
  // ポジショニング自体の評価における重み (posPoint: 0,0 ~ 1.0)
  float posPointRate;
  // 端よりに位置づけすることをどの程度重視するか (1.0 より上に設定)
  // used in evalAsteroForMove
  float edgeRate;
  float nowDistRate;

  /* init */
  int nextMoveIndex;
  
  /* getNextAction */
  int restAsteroidNum;

  /* finalize */
  int stageNumber = 0;


  /*-------------------------------------*/
  /* functions */
  /*-------------------------------------*/
  // local function
  // (0~250, 0~250) に座標変換
  Vector2 transUnder250(Vector2 v){
    float x = v.x; float y = v.y;
    if(x > 250)
      x = 500 - x;
    if(y > 250)
      y = 500 - y;
    return Vector2(x, y);
  }

  // move 用の評価関数
  // 端にいるほど優位になる
  // return: 0.0 ~ 1.0
  float evalAsteroForMove(Vector2 para_v){
    Vector2 v = transUnder250(para_v);
    
    float small = min(v.x, v.y);
    float large = max(v.x, v.y);

    float numer = small * edgeRate + large;
    float denom = 250 * edgeRate + 250;

    return numer / denom;
  }


  // 射撃のターゲット決める関数
  int decideShootTurget(const Stage& aStage){
    Vector2 myshipPos = aStage.ship().pos();
    
    int index = -1;
    float distSum = -10;

    // 同時に打てる惑星の数が最も多いパターンの中で、撃たれる星が船から最も遠くにあるものを選ぶ
    REP(i, aStage.asteroidCount()){
      if(aStage.asteroid(i).exists()) {
        // 今見る惑星のPos
        Vector2 asteroPos = aStage.asteroid(i).pos();
        Vector2 asteroPos00 = asteroPos - myshipPos;
        float nowDist = asteroPos.dist(myshipPos);

        // asteroPos との半直線をx座標に合わせる
        // 角度が知りたい
        float angle = Vector2(100, 0).angle(asteroPos00);
        if(asteroPos00.y < 0)
          angle = -angle;
        
        // x 座標が規定の範囲を守っているものの中から、半直線と円(惑星)の距離を取って撃てるかチェック                
        float kDistSum = nowDist * nowDistRate;
        REP(k, aStage.asteroidCount()){
          if(k != i && aStage.asteroid(k).exists()) {
            // x 座標が規定の範囲を守っているものの中から、半直線と円(惑星)の距離を取って撃てるかチェック
            Vector2 kPos00 = aStage.asteroid(k).pos() - myshipPos;
            Vector2 kPos00Rotated = kPos00.getRotatedRad(-angle);
            if(kPos00Rotated.x > 0){
              float asteroBeamDist = abs(kPos00Rotated.y);
              if(asteroBeamDist <= aStage.asteroid(k).radius()){
                kDistSum += kPos00Rotated.length();
              }
            }
          }
        }

        // 数が最も多いものを選ぶ
        if(kDistSum > distSum){
          distSum = kDistSum;
          index = i;
        }
      }
    }

    return index;
  }


  // 移動先の目的の惑星番号決める
  int decideMoveTurget(const Stage& aStage){
    // eval-check
    // 残り惑星数が少ない時、端移動の優先度下げる
    float restAsteroRate = 1.0;
    if(restAsteroidNum <= 5)
      restAsteroRate = 0.2;
    else if(restAsteroidNum <= 9){
      restAsteroRate = 0.86;
    }

    // main process
    int index = -1;
    float eval = 1e9;
    REP(i, aStage.asteroidCount()){
      if(aStage.asteroid(i).exists()) {
        Vector2 astero = aStage.asteroid(i).pos(); // 今見る惑星のPos
    
        float distNow = astero.dist(aStage.ship().pos());
        float posPoint = evalAsteroForMove(astero);

        float evalNow =
          distNow + (posPoint * posPointRate * restAsteroRate);
 
        if(evalNow < eval){
          eval = evalNow;
          index = i;
        }
      }
    }

    return index;
  }


  // Action phase 用の Move 目標決め関数
  // initialize phase では使えない
  Vector2 makeTargetMovePos(const Stage& aStage){
    if(aStage.asteroid(nextMoveIndex).exists())
      return aStage.asteroid(nextMoveIndex).pos();
    else{ // 新たに最も近い惑星を探す
      int index = decideMoveTurget(aStage);

      nextMoveIndex = index;
      return aStage.asteroid(nextMoveIndex).pos();
    }
  }



//------------------------------------------------------------------------------
/// Answer クラスのコンストラクタです。
///
/// @note ここにインスタンス生成時の処理を書くことができますが、何も書かなくても構いません。
Answer::Answer(){}

//------------------------------------------------------------------------------
/// Answer クラスのデストラクタです。
///
/// @note ここにインスタンス破棄時の処理を書くことができますが、何も書かなくても構いません。
Answer::~Answer(){}



//------------------------------------------------------------------------------
/// 各ステージ開始時に呼び出されます。
///
/// @note ここで、各ステージに対して初期処理を行うことができます。
///
/// @param[in] aStage 現在のステージ。
void Answer::init(const Stage& aStage)
{ 
  /*-------------------------------------*/
  /* initialize variables                */
  /*-------------------------------------*/
  // for evaluation
  // eval-check
  posPointRate = 179.5;
  edgeRate = 4.35;   // 移動時に端よりを目指すことをどの程度重視するか (1.0 より上に設定)
  nowDistRate = 0.815;  // 現在の予定移動先の射撃優先度

  // field variable
  restAsteroidNum = aStage.existingAsteroidCount();
  // fill nextMoveIndex
  nextMoveIndex = decideMoveTurget(aStage);
 

  /*-------------------------------------*/
  /* initial process                     */
  /*-------------------------------------*/

}



//------------------------------------------------------------------------------
/// 各ターンでの行動を返します。
///
/// @param[in] aStage 現在ステージの情報。
///
/// @return これから行う行動を表す Action クラス。
Action Answer::getNextAction(const Stage& aStage)
{
  restAsteroidNum = aStage.existingAsteroidCount();
  
  if(aStage.ship().canShoot()) { // レーザーが発射できるときは、レーザーを発射する。
    int index = decideShootTurget(aStage);

    // 発射目標にする小惑星
    Vector2 targetShootPos = aStage.asteroid(index).pos();
    return Action::Shoot(targetShootPos);
  }
  else {  // レーザーが発射できないときは、移動する。
    // 移動目標にする小惑星
    Vector2 targetMovePos = makeTargetMovePos(aStage);
    return Action::Move(targetMovePos);
  }
}


//------------------------------------------------------------------------------
/// 各ステージ終了時に呼び出されます。
///
/// @param[in] aStage 現在ステージの情報。
///
/// @note ここにステージ終了時の処理を書くことができますが、何も書かなくても構いません。
void Answer::finalize(const Stage& aStage)
{
  stageNumber++;
}

} // namespace
// EOF
