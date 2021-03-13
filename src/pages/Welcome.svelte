<script lang="ts">
  import { onMount } from "svelte";
  import dark from "smelte/src/dark";
  import api from "../api";
  import Container from "../templates/Container.svelte";
  import Menu from "../organisms/Menu.svelte";
  const darkMode = dark();

  onMount(async () => {
    const { inverted } = await api.display();
    darkMode.set(inverted);
  });
</script>

<main>
  <Container>
    <span slot="title">Welcome monophoto6</span>
    <Menu />
  </Container>
</main>
