<script lang="ts">
  import Container from "../templates/Container.svelte";
  import FileManager from "../organisms/FileManager.svelte";
</script>

<main>
  <Container>
    <span slot="title">File Management</span>
    <FileManager />
  </Container>
</main>
