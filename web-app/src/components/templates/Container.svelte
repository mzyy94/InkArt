<script>
  import { Button } from "smelte";

  function toHome() {
    location.hash = "#";
  }
</script>

<h1 class="text-center md:text-4xl text-2xl">
  <Button color="primary" icon="home" text light flat on:click={toHome} />
  <slot name="title">Title</slot>
</h1>
<section class="md:w-2/3 sm:w-auto md:m-auto m-3">
  <slot />
</section>
