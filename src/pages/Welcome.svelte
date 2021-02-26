<script lang="ts">
  import { onMount } from "svelte";
  import dark from "smelte/src/dark";
  import Container from "../templates/Container.svelte";
  import Menu from "../organisms/Menu.svelte";
  const darkMode = dark();

  onMount(() => {
    fetch("/status.json")
      .then((res) => res.json())
      .then(({ darkmode }) => {
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
