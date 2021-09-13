
interface Jetpack {
  minify(content: string): string,
  parse(content: string): any,
}

export default function(): Promise<Jetpack>;
