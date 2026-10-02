// test/chords.tsv 생성기 — scripts/gen-chords.sh 가 sebeol-type 소스 옆에 복사해서 실행한다
import { chordText } from './engine/semoe.ts';
import { SEMOE } from './layout/semoe.ts';
const EV: Record<string, number> = { KeyY:21,KeyU:22,KeyI:23,KeyO:24,KeyH:35,KeyJ:36,KeyK:37,KeyL:38,KeyN:49,KeyM:50,Semicolon:39,Period:52,KeyQ:16,KeyW:17,KeyE:18,KeyA:30,KeyS:31,KeyZ:44,KeyX:45,KeyR:19,KeyT:20,KeyD:32,KeyF:33,KeyG:34,KeyC:46,KeyV:47,KeyB:48,KeyP:25,Comma:51,Space:57,Digit1:2 };
const keys = Object.keys(SEMOE);
const out: string[] = [];
const rec = (start: number, cur: string[], max: number) => {
  if (cur.length) out.push(cur.map(k => EV[k]).join(',') + '\t' + chordText(cur).replace('?', ''));
  if (cur.length === max) return;
  for (let i = start; i < keys.length; i++) rec(i + 1, [...cur, keys[i]], max);
};
rec(0, [], 5);
// 문장부호·스페이스 섞기
for (const extra of [['KeyP'], ['Comma'], ['Space'], ['Comma', 'Space'], ['Digit1']])
  for (const c of [['KeyK','KeyF'], ['KeyJ','KeyD','Semicolon'], ['KeyK','Period','KeyF','KeyX'], []])
    if (c.length || extra.length) out.push([...c, ...extra].map(k => EV[k]).join(',') + '\t' + chordText([...c, ...extra]).replace('?', ''));
process.stdout.write(out.join('\n') + '\n');
