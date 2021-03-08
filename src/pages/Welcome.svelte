<script lang="ts">
  import { onMount } from "svelte";
  import dark from "smelte/src/dark";
  import { get } from "../api/method";
  import Container from "../templates/Container.svelte";
  import Menu from "../organisms/Menu.svelte";
  const darkMode = dark();

  onMount(() => {
    get<{ darkmode: boolean }>("/status.json").then(({ darkmode }) => {
      darkMode.set(darkmode);
    });
  });
</script>

<main>
  <Container>
    <span slot="title">Welcome monophoto6</span>
    <Menu />
  </Container>
</main>
