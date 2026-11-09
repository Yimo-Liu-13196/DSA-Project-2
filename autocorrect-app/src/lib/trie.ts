/**
 * TypeScript port of the backend `Trie` (Trie.h / Trie.cpp from DSA-Project-2).
 * Same rules as the C++:
 *  - letters are stored lowercase; only a-z, apostrophe (') and hyphen (-) are allowed
 *  - an apostrophe/hyphen must follow a letter (no leading or doubled separators)
 *  - a finished word cannot end in a separator, but a *prefix* may (e.g. "don-")
 *  - complete() ranks by frequency (highest first), ties broken alphabetically
 *  - frequencies are unsigned 32-bit and must be positive
 */

const ALPHABET = 28; // a-z, ', -
const UINT_MAX = 4294967295;

/** Mirrors std::invalid_argument. */
export class InvalidWordError extends Error {}
/** Mirrors std::overflow_error. */
export class FrequencyOverflowError extends Error {}

class Node {
  child: (Node | null)[] = new Array<Node | null>(ALPHABET).fill(null);
  isWord = false;
  freq = 0;
}

function charIdx(c: string): number {
  if (c >= "a" && c <= "z") return c.charCodeAt(0) - 97;
  if (c === "'") return 26;
  if (c === "-") return 27;
  return -1;
}

function idxChar(idx: number): string {
  if (idx >= 0 && idx < 26) return String.fromCharCode(97 + idx);
  return idx === 26 ? "'" : "-";
}

function norm(text: string, allowTrailing = false): string {
  if (text.length === 0) throw new InvalidWordError("word must contain letters");

  let res = "";
  let lastSep = false;

  for (let i = 0; i < text.length; i++) {
    let c = text[i];
    if (c >= "A" && c <= "Z") c = String.fromCharCode(c.charCodeAt(0) + 32);

    if (c >= "a" && c <= "z") {
      lastSep = false;
    } else if (c === "'" || c === "-") {
      if (i === 0 || lastSep) throw new InvalidWordError("apostrophes and hyphens must follow a letter");
      lastSep = true;
    } else {
      throw new InvalidWordError("word must contain only ascii letters, apostrophes, and hyphens");
    }
    res += c;
  }

  if (lastSep && !allowTrailing) throw new InvalidWordError("word must end with a letter");
  return res;
}

export class Trie {
  private root = new Node();
  private count = 0;

  insert(word: string, startFreq = 1): boolean {
    const w = norm(word);
    if (startFreq === 0) throw new InvalidWordError("initial frequency must be positive");

    let cur = this.root;
    for (const ch of w) {
      const idx = charIdx(ch);
      cur = cur.child[idx] ??= new Node();
    }
    if (cur.isWord) return false;

    cur.isWord = true;
    cur.freq = startFreq;
    this.count++;
    return true;
  }

  contains(word: string): boolean {
    const node = this.findNode(norm(word));
    return node !== null && node.isWord;
  }

  incrementFrequency(word: string, amount = 1): boolean {
    const w = norm(word);
    if (amount === 0) throw new InvalidWordError("frequency increment must be positive");

    const node = this.findNode(w);
    if (node === null || !node.isWord) return false;
    if (amount > UINT_MAX - node.freq) throw new FrequencyOverflowError("frequency would overflow");

    node.freq += amount;
    return true;
  }

  get size(): number {
    return this.count;
  }

  /** Up to `limit` words starting with `prefix`, most frequent first. Throws InvalidWordError on a bad prefix. */
  complete(prefix: string, limit: number): string[] {
    if (prefix.length === 0 || limit === 0) return [];

    const p = norm(prefix, true);
    const start = this.findNode(p);
    if (start === null) return [];

    const words: { word: string; freq: number }[] = [];
    this.gatherWords(start, p, words);

    words.sort((a, b) => {
      if (a.freq !== b.freq) return b.freq - a.freq;
      return a.word < b.word ? -1 : a.word > b.word ? 1 : 0;
    });

    return words.slice(0, limit).map((w) => w.word);
  }

  private findNode(key: string): Node | null {
    let cur = this.root;
    for (const ch of key) {
      const idx = charIdx(ch);
      const next = idx === -1 ? null : cur.child[idx];
      if (next === null) return null;
      cur = next;
    }
    return cur;
  }

  private gatherWords(node: Node, cur: string, words: { word: string; freq: number }[]) {
    if (node.isWord) words.push({ word: cur, freq: node.freq });
    for (let i = 0; i < ALPHABET; i++) {
      const next = node.child[i];
      if (next) this.gatherWords(next, cur + idxChar(i), words);
    }
  }
}
