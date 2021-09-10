
interface Jetpack {
  minify(content: string): string,
}

export default function(): Promise<Jetpack>;
